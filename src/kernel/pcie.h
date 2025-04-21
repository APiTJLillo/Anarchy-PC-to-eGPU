#ifndef ANARCHY_PCIE_H
#define ANARCHY_PCIE_H

#include "anarchy-egpu.h"
#include <linux/pci.h>

/* PCIe timing parameters */
#define PCIE_LINK_TRAINING_TIMEOUT_MS 100
#define PCIE_LINK_RETRY_COUNT 3
#define PCIE_LINK_RETRY_DELAY_MS 50

/* Additional PCIe timing parameters */
#define TB_PCIE_POWER_TIMEOUT_MS 5000
#define TB_PCIE_POWER_CHECK_MS 100
#define TB_PCIE_HOST_DEBOUNCE_MS 2500
#define TB_PCIE_MODE_CHECK_MS 50
#define TB_PCIE_POWER_TRANSITION_MS 1000
#define TB_PCIE_MODE_SETTLE_MS 500
#define TB_PCIE_MAX_MODE_RETRIES 5
#define TB_PCIE_RETRY_BASE_MS 100
#define TB_PCIE_LINK_SETTLE_MS 100

/* PCIe link status bits */
#define PCIE_LINK_STATUS_UP (PCI_EXP_LNKSTA_DLLLA)
#define PCIE_LINK_STATUS_TRAINING (PCI_EXP_LNKSTA_LT)
#define PCIE_LINK_STATUS_DLL_ACTIVE (PCI_EXP_LNKSTA_DLLLA)

/* PCIe device class */
#define PCI_CLASS_VGA 0x0300

/* Function prototypes */
int anarchy_pcie_setup_bars(struct anarchy_device *adev);
int anarchy_pcie_setup_capabilities(struct anarchy_device *adev);
int anarchy_pcie_read_config(struct anarchy_device *adev, int where, int size, u32 *val);
int anarchy_pcie_write_config(struct anarchy_device *adev, int where, int size, u32 val);
void anarchy_pcie_retrain_link(struct anarchy_device *adev);
void anarchy_pcie_handle_error(struct anarchy_device *adev, int error);
void anarchy_pcie_recovery_work(struct work_struct *work);
void anarchy_pcie_check_link(struct anarchy_device *adev);
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);
void anarchy_pcie_cleanup(struct anarchy_device *adev);
int anarchy_pcie_check_link_config(struct anarchy_device *adev);
int anarchy_pcie_tb_recovery(struct anarchy_device *adev);
int anarchy_pcie_wait_for_link(struct anarchy_device *adev, u32 *link_status);

/* Helper functions */
static inline void pcie_reset_link(struct pci_dev *pdev)
{
    u16 lnk_ctrl;
    pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &lnk_ctrl);
    lnk_ctrl |= PCI_EXP_LNKCTL_RL;
    pcie_capability_write_word(pdev, PCI_EXP_LNKCTL, lnk_ctrl);
}

static inline int pcie_wait_link_training(struct pci_dev *pdev)
{
    int timeout = PCIE_LINK_TRAINING_TIMEOUT_MS;
    u16 lnk_status;

    while (timeout > 0) {
        pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
        if (!(lnk_status & PCI_EXP_LNKSTA_LT))
            return 0;
        msleep(1);
        timeout--;
    }
    return -ETIMEDOUT;
}

static inline bool pcie_link_is_up(struct pci_dev *pdev)
{
    u16 lnk_status;
    pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnk_status);
    return (lnk_status & PCIE_LINK_STATUS_UP) == PCIE_LINK_STATUS_UP;
}

#endif /* ANARCHY_PCIE_H */