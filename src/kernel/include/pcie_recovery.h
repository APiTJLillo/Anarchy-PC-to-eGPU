#ifndef ANARCHY_PCIE_RECOVERY_H
#define ANARCHY_PCIE_RECOVERY_H

#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/jiffies.h>
#include "pcie_types.h"
#include "anarchy_device.h"

/* PCIe recovery states */
#define ANARCHY_PCIE_STATE_NORMAL    4
#define ANARCHY_PCIE_STATE_RECOVERY  5
#define ANARCHY_PCIE_STATE_LINK_DOWN 6

/* PCIe recovery constants */
#define ANARCHY_PCIE_MAX_RETRIES     3
#define ANARCHY_PCIE_ERR_SPEED_DOWNGRADE  100
#define ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE  101

/* PCIe recovery state */
struct anarchy_pcie_recovery {
    spinlock_t recovery_lock;
    atomic_t retries;
    struct work_struct recovery_work;
    unsigned long last_down;
};

/* Function declarations */
void anarchy_pcie_recovery_init(struct anarchy_device *adev);
void anarchy_pcie_recovery_cleanup(struct anarchy_device *adev);
int anarchy_pcie_train_link(struct anarchy_device *adev);

#endif /* ANARCHY_PCIE_RECOVERY_H */ 