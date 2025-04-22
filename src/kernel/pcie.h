#ifndef _ANARCHY_KERNEL_PCIE_H_
#define _ANARCHY_KERNEL_PCIE_H_

#include <linux/pci.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include "../../include/anarchy-pcie.h"

/* PCIe error codes */
#define ANARCHY_PCIE_ERR_TIMEOUT -ETIMEDOUT
#define ANARCHY_PCIE_ERR_LINK_DOWN -ENOLINK
#define ANARCHY_PCIE_ERR_DEVICE_NOT_FOUND -ENODEV
#define ANARCHY_PCIE_ERR_RETRY -EAGAIN
#define ANARCHY_PCIE_ERR_TRAINING_FAILED -EAGAIN
#define ANARCHY_PCIE_ERR_SPEED_DOWNGRADE -EINVAL
#define ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE -EINVAL

/* Enhanced recovery parameters */
#define ANARCHY_PCIE_MAX_RETRIES 5
#define ANARCHY_PCIE_LINK_TIMEOUT_MS 1000

/* PCIe training wait queue */
extern wait_queue_head_t pcie_training_wait;

/* Function declarations */
bool pcie_link_is_up(struct pci_dev *pdev);
void pcie_reset_link(struct pci_dev *pdev);
int pcie_wait_link_training(struct pci_dev *pdev);

#endif /* _ANARCHY_KERNEL_PCIE_H_ */