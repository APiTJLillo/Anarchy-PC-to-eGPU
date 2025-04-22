#ifndef ANARCHY_PCIE_STATE_H
#define ANARCHY_PCIE_STATE_H

#include <linux/pci.h>
#include <linux/workqueue.h>
#include "forward.h"
#include "pcie_types.h"

/* PCIe link states */
enum anarchy_pcie_link_state {
    ANARCHY_PCIE_STATE_UNKNOWN = 0,
    ANARCHY_PCIE_STATE_NORMAL,
    ANARCHY_PCIE_STATE_LINK_DOWN,
    ANARCHY_PCIE_STATE_ERROR,
    ANARCHY_PCIE_STATE_RECOVERY,
    ANARCHY_PCIE_STATE_TRAINING
};

/* PCIe error types */
enum anarchy_pcie_error_type {
    ANARCHY_PCIE_ERR_NONE = 0,
    ANARCHY_PCIE_ERR_LINK_DOWN = 1,
    ANARCHY_PCIE_ERR_TRAINING = 2,
    ANARCHY_PCIE_ERR_SPEED = 3,
    ANARCHY_PCIE_ERR_WIDTH = 4,
    ANARCHY_PCIE_ERR_TIMEOUT = 5
};

/* PCIe state tracking */
struct anarchy_pcie_state {
    /* Hardware state */
    struct pci_dev *nhi;
    enum anarchy_pcie_link_state state;
    enum anarchy_pcie_speed link_speed;
    enum anarchy_pcie_width link_width;
    
    /* Error handling */
    spinlock_t recovery_lock;
    atomic_t retries;
    unsigned long last_recovery;
    struct work_struct recovery_work;
    
    /* Back pointer to parent device */
    struct anarchy_device *adev;
    
    /* Stats tracking */
    u32 training_attempts;
    u32 successful_trains;
    u64 total_uptime;
    unsigned long last_up;
    unsigned long last_down;
};

#endif /* ANARCHY_PCIE_STATE_H */
};

/* PCIe state management */
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);
void anarchy_pcie_disable(struct anarchy_device *adev);
int anarchy_pcie_train_link(struct anarchy_device *adev);
void anarchy_pcie_handle_error(struct anarchy_device *adev, 
                              enum anarchy_pcie_error_type error);

#endif /* ANARCHY_PCIE_STATE_H */
