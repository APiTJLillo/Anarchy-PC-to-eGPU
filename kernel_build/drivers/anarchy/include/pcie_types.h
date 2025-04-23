#ifndef ANARCHY_PCIE_TYPES_H
#define ANARCHY_PCIE_TYPES_H

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include "pcie_forward.h"

/* PCIe constants */
#define ANARCHY_PCIE_MAX_LANES  16
#define ANARCHY_PCIE_MAX_SPEED  ANARCHY_PCIE_SPEED_32GT  /* RTX 4090 max PCIe 5.0 */

/* PCIe link width values */
#define ANARCHY_PCIE_x1   ANARCHY_PCIE_WIDTH_x1
#define ANARCHY_PCIE_x2   ANARCHY_PCIE_WIDTH_x2
#define ANARCHY_PCIE_x4   ANARCHY_PCIE_WIDTH_x4
#define ANARCHY_PCIE_x8   ANARCHY_PCIE_WIDTH_x8
#define ANARCHY_PCIE_x16  ANARCHY_PCIE_WIDTH_x16

/* PCIe generation aliases */
#define ANARCHY_PCIE_GEN1  ANARCHY_PCIE_SPEED_2_5GT
#define ANARCHY_PCIE_GEN2  ANARCHY_PCIE_SPEED_5GT
#define ANARCHY_PCIE_GEN3  ANARCHY_PCIE_SPEED_8GT
#define ANARCHY_PCIE_GEN4  ANARCHY_PCIE_SPEED_16GT
#define ANARCHY_PCIE_GEN5  ANARCHY_PCIE_SPEED_32GT
#define ANARCHY_PCIE_GEN6  ANARCHY_PCIE_SPEED_64GT

/* PCIe recovery state */
struct anarchy_pcie_recovery {
    spinlock_t recovery_lock;
    atomic_t retries;
    struct work_struct recovery_work;
    unsigned long last_down;
};

/* PCIe link state structure */
struct anarchy_pcie_state {
    struct anarchy_device *adev;       /* Parent device */
    struct pci_dev *pdev;              /* PCIe device */
    enum anarchy_pcie_link_state state;/* Current link state */
    enum anarchy_pcie_speed speed;     /* Current link speed */
    u32 link_width;                    /* Current link width */
    u32 max_payload_size;              /* Max payload size in bytes */
    u32 max_read_req_size;            /* Max read request size in bytes */
    bool enabled;                      /* Link enabled flag */
    bool needs_retrain;               /* Link needs retraining flag */
    u32 error_count;                  /* Link error counter */
    enum anarchy_pcie_error_type last_error; /* Last error type */
    struct anarchy_pcie_recovery recovery;  /* Recovery state */
};

#endif /* ANARCHY_PCIE_TYPES_H */
