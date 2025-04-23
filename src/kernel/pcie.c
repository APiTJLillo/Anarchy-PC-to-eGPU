#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include "include/types.h"
#include "include/common.h"
#include "include/pcie_forward.h"
#include "include/pcie_types.h"
#include "include/pcie_state.h"
#include "include/anarchy_device.h"
#include "pcie.h"

/* PCI Express Link Status register bits */
#define PCI_EXP_LNKSTA_DLLLA    0x2000  /* Data Link Layer Link Active */

/* Forward declarations */
static void anarchy_pcie_recovery_work(struct work_struct *work);

static int pcie_set_link_speed(struct anarchy_device *adev, enum anarchy_pcie_speed speed)
{
    struct pci_dev *pdev = adev->pdev;
    u16 lnk_ctrl2;
    int ret;

    ret = pcie_capability_read_word(pdev, PCI_EXP_LNKCTL2, &lnk_ctrl2);
    if (ret)
        return ret;

    lnk_ctrl2 &= ~PCI_EXP_LNKCTL2_TLS;
    lnk_ctrl2 |= speed;
    
    return pcie_capability_write_word(pdev, PCI_EXP_LNKCTL2, lnk_ctrl2);
}

static int pcie_set_link_width(struct anarchy_device *adev, enum anarchy_pcie_width width)
{
    struct pci_dev *pdev = adev->pdev;
    u16 lnk_ctrl;
    int ret;

    ret = pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
    if (ret)
        return ret;

    /* Width is read-only, just verify it */
    if ((lnk_ctrl & PCI_EXP_LNKSTA_NLW) >> 4 < width)
        return -EINVAL;

    return 0;
}

static int anarchy_pcie_check_link_config(struct anarchy_device *adev)
{
    struct pci_dev *pdev = adev->pdev;
    u16 lnk_stat;
    int ret;

    ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_stat);
    if (ret)
        return ret;

    adev->pcie_state.speed = (lnk_stat & PCI_EXP_LNKSTA_CLS) >> 0;
    adev->pcie_state.link_width = (lnk_stat & PCI_EXP_LNKSTA_NLW) >> 4;

    if (adev->pcie_state.speed < ANARCHY_PCIE_GEN4)
        return ANARCHY_PCIE_ERR_SPEED_DOWNGRADE;

    if (adev->pcie_state.link_width < ANARCHY_PCIE_x8)
        return ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE;

    return 0;
}

int anarchy_pcie_init(struct anarchy_device *adev)
{
    int ret;

    /* Initialize PCIe state */
    ret = anarchy_pcie_init_state(adev);
    if (ret)
        return ret;

    /* Enable PCIe link */
    ret = anarchy_pcie_enable_link(adev);
    if (ret) {
        anarchy_pcie_cleanup_state(adev);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_pcie_init);

void anarchy_pcie_exit(struct anarchy_device *adev)
{
    if (!adev)
        return;

    /* Disable PCIe link */
    anarchy_pcie_disable(adev);

    /* Cleanup PCIe state */
    anarchy_pcie_cleanup_state(adev);
}
EXPORT_SYMBOL_GPL(anarchy_pcie_exit);

void anarchy_pcie_disable(struct anarchy_device *adev)
{
    if (!adev)
        return;

    /* Disable PCIe link */
    anarchy_pcie_disable_link(adev);

    /* Wait for link to go down */
    msleep(100);
}
EXPORT_SYMBOL_GPL(anarchy_pcie_disable);

void anarchy_pcie_disable_link(struct anarchy_device *adev)
{
    unsigned long flags;

    spin_lock_irqsave(&adev->pcie_state.recovery.recovery_lock, flags);
    adev->pcie_state.state = ANARCHY_PCIE_STATE_LINK_DOWN;
    adev->pcie_state.recovery.last_down = jiffies;
    atomic_set(&adev->pcie_state.recovery.retries, 0);
    spin_unlock_irqrestore(&adev->pcie_state.recovery.recovery_lock, flags);
}

static void anarchy_pcie_recovery_work(struct work_struct *work)
{
    struct anarchy_pcie_recovery *recovery = container_of(work,
                                                        struct anarchy_pcie_recovery,
                                                        recovery_work);
    struct anarchy_pcie_state *pcie = container_of(recovery,
                                                  struct anarchy_pcie_state,
                                                  recovery);
    struct anarchy_device *adev = pcie->adev;
    int ret;

    /* First try standard PCIe recovery */
    ret = pcie_set_link_speed(adev, ANARCHY_PCIE_GEN4);
    if (ret)
        goto try_tb_recovery;

    ret = pcie_set_link_width(adev, ANARCHY_PCIE_x8);
    if (ret)
        goto try_tb_recovery;

    /* Wait for link */
    msleep(100);

    if (pcie_link_is_up(adev->pdev)) {
        ret = anarchy_pcie_check_link_config(adev);
        if (ret == 0) {
            pcie->state = ANARCHY_PCIE_STATE_NORMAL;
            return;
        }
    }

try_tb_recovery:
    /* If standard recovery failed, try Thunderbolt-specific recovery */
    if (tb_service_reset(adev->service)) {
        msleep(500);
        ret = anarchy_pcie_train_link(adev);
        if (ret == 0)
            return;
    }

    /* Recovery failed */
    pcie->state = ANARCHY_PCIE_STATE_ERROR;
}

void anarchy_pcie_handle_error(struct anarchy_device *adev,
                             enum anarchy_pcie_error_type error)
{
    struct anarchy_pcie_state *pcie = &adev->pcie_state;
    unsigned long flags;

    spin_lock_irqsave(&pcie->recovery.recovery_lock, flags);

    if (atomic_read(&pcie->recovery.retries) >= ANARCHY_PCIE_MAX_RETRIES) {
        pcie->state = ANARCHY_PCIE_STATE_ERROR;
        spin_unlock_irqrestore(&pcie->recovery.recovery_lock, flags);
        return;
    }

    atomic_inc(&pcie->recovery.retries);
    pcie->state = ANARCHY_PCIE_STATE_RECOVERY;
    schedule_work(&pcie->recovery.recovery_work);

    spin_unlock_irqrestore(&pcie->recovery.recovery_lock, flags);
}

int anarchy_pcie_train_link(struct anarchy_device *adev)
{
    struct pci_dev *pdev = adev->pdev;
    u16 lnk_ctrl;
    int ret;

    /* Set link state to training */
    adev->pcie_state.state = ANARCHY_PCIE_STATE_TRAINING;

    /* Read link control register */
    ret = pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
    if (ret)
        return ret;

    /* Initiate link training */
    lnk_ctrl |= PCI_EXP_LNKCTL_RL;
    ret = pcie_capability_write_word(pdev, PCI_EXP_LNKCTL, lnk_ctrl);
    if (ret)
        return ret;

    /* Wait for link training to complete */
    msleep(100);

    /* Check link status */
    ret = anarchy_pcie_check_link_config(adev);
    if (ret) {
        adev->pcie_state.state = ANARCHY_PCIE_STATE_ERROR;
        return ret;
    }

    adev->pcie_state.state = ANARCHY_PCIE_STATE_ACTIVE;
    return 0;
}

bool pcie_link_is_up(struct pci_dev *pdev)
{
    u16 lnk_stat;
    int ret;

    ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_stat);
    if (ret)
        return false;

    return (lnk_stat & PCI_EXP_LNKSTA_DLLLA) != 0;
}
EXPORT_SYMBOL_GPL(pcie_link_is_up);

int anarchy_pcie_optimize_settings(struct anarchy_device *adev)
{
    // ... existing implementation ...
}
EXPORT_SYMBOL_GPL(anarchy_pcie_optimize_settings);

void anarchy_pcie_cleanup_state(struct anarchy_device *adev)
{
    if (!adev)
        return;

    /* Cancel any pending work */
    cancel_work_sync(&adev->pcie_state.recovery.recovery_work);

    /* Reset PCIe state */
    adev->pcie_state.state = ANARCHY_PCIE_STATE_UNKNOWN;
    adev->pcie_state.speed = ANARCHY_PCIE_GEN1;
    adev->pcie_state.link_width = ANARCHY_PCIE_x1;
}
EXPORT_SYMBOL_GPL(anarchy_pcie_cleanup_state);

EXPORT_SYMBOL_GPL(anarchy_pcie_handle_error);

int anarchy_pcie_init_state(struct anarchy_device *adev)
{
    // ... existing implementation ...
}
EXPORT_SYMBOL_GPL(anarchy_pcie_init_state);

int anarchy_pcie_enable_link(struct anarchy_device *adev)
{
    struct pci_dev *pdev = adev->pdev;
    u16 lnk_ctrl;
    int ret;

    /* Set link state to training */
    adev->pcie_state.state = ANARCHY_PCIE_STATE_TRAINING;

    /* Read link control register */
    ret = pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
    if (ret)
        return ret;

    /* Enable link */
    lnk_ctrl &= ~PCI_EXP_LNKCTL_LD;
    ret = pcie_capability_write_word(pdev, PCI_EXP_LNKCTL, lnk_ctrl);
    if (ret)
        return ret;

    /* Wait for link to stabilize */
    msleep(100);

    /* Check link status */
    ret = anarchy_pcie_check_link_config(adev);
    if (ret) {
        adev->pcie_state.state = ANARCHY_PCIE_STATE_ERROR;
        return ret;
    }

    adev->pcie_state.state = ANARCHY_PCIE_STATE_ACTIVE;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_pcie_enable_link);
