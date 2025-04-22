#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include "include/types.h"
#include "include/common.h"
#include "include/pcie_types.h"
#include "include/anarchy_device.h"
#include "pcie.h"

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

    adev->pcie_state.link_speed = (lnk_stat & PCI_EXP_LNKSTA_CLS) >> 0;
    adev->pcie_state.link_width = (lnk_stat & PCI_EXP_LNKSTA_NLW) >> 4;

    if (adev->pcie_state.link_speed < ANARCHY_PCIE_GEN4)
        return ANARCHY_PCIE_ERR_SPEED_DOWNGRADE;

    if (adev->pcie_state.link_width < ANARCHY_PCIE_x8)
        return ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE;

    return 0;
}

int anarchy_pcie_init(struct anarchy_device *adev)
{
    /* Initialize state */
    adev->pcie_state.state = ANARCHY_PCIE_STATE_UNKNOWN;
    adev->pcie_state.link_speed = ANARCHY_PCIE_GEN1;
    adev->pcie_state.link_width = ANARCHY_PCIE_x1;
    adev->pcie_state.training_attempts = 0;
    adev->pcie_state.successful_trains = 0;
    adev->pcie_state.adev = adev;

    spin_lock_init(&adev->pcie_state.recovery_lock);
    atomic_set(&adev->pcie_state.retries, 0);
    INIT_WORK(&adev->pcie_state.recovery_work, anarchy_pcie_recovery_work);

    return 0;
}

void anarchy_pcie_exit(struct anarchy_device *adev)
{
    /* Cancel any pending work */
    cancel_work_sync(&adev->pcie_state.recovery_work);

    /* Disable PCIe link */
    anarchy_pcie_disable(adev);
}

void anarchy_pcie_disable(struct anarchy_device *adev)
{
    unsigned long flags;

    spin_lock_irqsave(&adev->pcie_state.recovery_lock, flags);
    adev->pcie_state.state = ANARCHY_PCIE_STATE_LINK_DOWN;
    adev->pcie_state.last_down = jiffies;
    atomic_set(&adev->pcie_state.retries, 0);
    spin_unlock_irqrestore(&adev->pcie_state.recovery_lock, flags);
}

static void anarchy_pcie_recovery_work(struct work_struct *work)
{
    struct anarchy_pcie_state *pcie = container_of(work,
                                                  struct anarchy_pcie_state,
                                                  recovery_work);
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

    spin_lock_irqsave(&pcie->recovery_lock, flags);

    if (atomic_read(&pcie->retries) >= ANARCHY_PCIE_MAX_RETRIES) {
        pcie->state = ANARCHY_PCIE_STATE_ERROR;
        spin_unlock_irqrestore(&pcie->recovery_lock, flags);
        return;
    }

    atomic_inc(&pcie->retries);
    pcie->state = ANARCHY_PCIE_STATE_RECOVERY;
    schedule_work(&pcie->recovery_work);

    spin_unlock_irqrestore(&pcie->recovery_lock, flags);
}

EXPORT_SYMBOL_GPL(anarchy_pcie_init);
EXPORT_SYMBOL_GPL(anarchy_pcie_exit);
EXPORT_SYMBOL_GPL(anarchy_pcie_disable);
EXPORT_SYMBOL_GPL(anarchy_pcie_handle_error);
