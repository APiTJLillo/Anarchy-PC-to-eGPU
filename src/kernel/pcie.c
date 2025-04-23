#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include "include/anarchy_device.h"
#include "include/common.h"
#include "include/pcie_types.h"
#include "include/pcie_forward.h"
#include "include/pcie_recovery.h"
#include "include/pcie_state.h"
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
    struct anarchy_device *adev = container_of(work, struct anarchy_device,
                                             pcie_state.recovery.recovery_work);
    int ret;

    /* Attempt PCIe link recovery */
    ret = anarchy_pcie_train_link(adev);
    if (ret)
        goto try_tb_recovery;

    /* Wait for link */
    msleep(100);

    if (pcie_link_is_up(adev->pdev)) {
        ret = anarchy_pcie_check_link_config(adev);
        if (ret == 0) {
            adev->pcie_state.state = ANARCHY_PCIE_STATE_NORMAL;
            return;
        }
    }

try_tb_recovery:
    /* If standard recovery failed, try Thunderbolt-specific recovery */
    ret = tb_service_reset(adev->service);
    if (ret) {
        dev_err(adev->dev, "Thunderbolt service reset failed: %d\n", ret);
        goto recovery_failed;
    }

    /* Wait for Thunderbolt link to stabilize */
    msleep(500);

    /* Try PCIe link training again */
    ret = anarchy_pcie_train_link(adev);
    if (ret == 0) {
        if (pcie_link_is_up(adev->pdev)) {
            ret = anarchy_pcie_check_link_config(adev);
            if (ret == 0) {
                adev->pcie_state.state = ANARCHY_PCIE_STATE_NORMAL;
                return;
            }
        }
    }

recovery_failed:
    /* All recovery attempts failed */
    dev_err(adev->dev, "PCIe recovery failed after Thunderbolt reset\n");
    adev->pcie_state.state = ANARCHY_PCIE_STATE_ERROR;
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
    struct pci_dev *pdev = adev->pdev;
    u16 devctl;
    int ret;

    if (!pdev)
        return -EINVAL;

    /* Configure PCIe link for maximum performance */
    ret = pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S | PCIE_LINK_STATE_L1 |
                                PCIE_LINK_STATE_CLKPM);
    if (ret)
        dev_warn(&pdev->dev, "Failed to disable PCIe power management: %d\n", ret);

    /* Read current device control register */
    ret = pcie_capability_read_word(pdev, PCI_EXP_DEVCTL, &devctl);
    if (ret) {
        dev_warn(&pdev->dev, "Failed to read PCIe device control: %d\n", ret);
        return ret;
    }

    /* Enable performance features:
     * - Maximum read request size
     * - Extended tags 
     * - Relaxed ordering
     * - No snoop
     */
    devctl |= (PCI_EXP_DEVCTL_READRQ | PCI_EXP_DEVCTL_EXT_TAG |
               PCI_EXP_DEVCTL_RELAX_EN | PCI_EXP_DEVCTL_NOSNOOP_EN);

    ret = pcie_capability_write_word(pdev, PCI_EXP_DEVCTL, devctl);
    if (ret)
        dev_warn(&pdev->dev, "Failed to write PCIe device control: %d\n", ret);

    return ret;
}

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
    if (!adev)
        return -EINVAL;

    /* Initialize PCIe state */
    adev->pcie_state.state = ANARCHY_PCIE_STATE_INIT;
    adev->pcie_state.speed = ANARCHY_PCIE_GEN1;
    adev->pcie_state.link_width = ANARCHY_PCIE_x1;

    /* Initialize recovery work */
    INIT_WORK(&adev->pcie_state.recovery.recovery_work,
              anarchy_pcie_recovery_work);

    /* Check initial link configuration */
    return anarchy_pcie_check_link_config(adev);
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

u64 anarchy_pcie_get_bandwidth_usage(struct anarchy_device *adev)
{
    u32 link_width, link_speed;
    u64 bandwidth = 0;

    if (!adev || !adev->pdev)
        return 0;

    /* Get current link width and speed */
    pcie_capability_read_dword(adev->pdev, PCI_EXP_LNKSTA, &link_width);
    link_width = (link_width & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;

    pcie_capability_read_dword(adev->pdev, PCI_EXP_LNKSTA, &link_speed);
    link_speed = link_speed & PCI_EXP_LNKSTA_CLS;

    /* Calculate bandwidth in bytes/sec */
    switch (link_speed) {
    case 1: /* 2.5 GT/s */
        bandwidth = 250000000ULL; /* 250 MB/s per lane */
        break;
    case 2: /* 5.0 GT/s */
        bandwidth = 500000000ULL; /* 500 MB/s per lane */
        break;
    case 3: /* 8.0 GT/s */
        bandwidth = 984600000ULL; /* ~985 MB/s per lane */
        break;
    case 4: /* 16.0 GT/s */
        bandwidth = 1969000000ULL; /* ~1.97 GB/s per lane */
        break;
    default:
        return 0;
    }

    return bandwidth * link_width;
}
EXPORT_SYMBOL_GPL(anarchy_pcie_get_bandwidth_usage);
