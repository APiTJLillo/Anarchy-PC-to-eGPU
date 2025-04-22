#ifndef ANARCHY_PCIE_TYPES_H
#define ANARCHY_PCIE_TYPES_H

#include <linux/types.h>
#include <linux/pci.h>
#include "forward.h"

/* PCIe link states */
enum anarchy_pcie_link_state {
    ANARCHY_PCIE_STATE_UNKNOWN = 0,
    ANARCHY_PCIE_STATE_DOWN,
    ANARCHY_PCIE_STATE_TRAINING,
    ANARCHY_PCIE_STATE_ACTIVE,
    ANARCHY_PCIE_STATE_ERROR
};

/* PCIe link speeds */
enum anarchy_pcie_speed {
    ANARCHY_PCIE_SPEED_UNKNOWN = 0,
    ANARCHY_PCIE_SPEED_2_5GT,   /* PCIe 1.0 */
    ANARCHY_PCIE_SPEED_5GT,     /* PCIe 2.0 */
    ANARCHY_PCIE_SPEED_8GT,     /* PCIe 3.0 */
    ANARCHY_PCIE_SPEED_16GT,    /* PCIe 4.0 */
    ANARCHY_PCIE_SPEED_32GT,    /* PCIe 5.0 */
    ANARCHY_PCIE_SPEED_64GT     /* PCIe 6.0 */
};

/* PCIe constants */
#define ANARCHY_PCIE_MAX_LANES  16
#define ANARCHY_PCIE_MAX_SPEED  ANARCHY_PCIE_SPEED_32GT  /* RTX 4090 max PCIe 5.0 */

/* PCIe link width values */
#define ANARCHY_PCIE_x1   1
#define ANARCHY_PCIE_x2   2
#define ANARCHY_PCIE_x4   4
#define ANARCHY_PCIE_x8   8
#define ANARCHY_PCIE_x16  16

/* PCIe generation aliases */
#define ANARCHY_PCIE_GEN1  ANARCHY_PCIE_SPEED_2_5GT
#define ANARCHY_PCIE_GEN2  ANARCHY_PCIE_SPEED_5GT
#define ANARCHY_PCIE_GEN3  ANARCHY_PCIE_SPEED_8GT
#define ANARCHY_PCIE_GEN4  ANARCHY_PCIE_SPEED_16GT
#define ANARCHY_PCIE_GEN5  ANARCHY_PCIE_SPEED_32GT
#define ANARCHY_PCIE_GEN6  ANARCHY_PCIE_SPEED_64GT

/* PCIe error types */
enum anarchy_pcie_error_type {
    ANARCHY_PCIE_ERR_NONE = 0,
    ANARCHY_PCIE_ERR_TRAINING,
    ANARCHY_PCIE_ERR_LINK_DOWN,
    ANARCHY_PCIE_ERR_TIMEOUT,
    ANARCHY_PCIE_ERR_CORRECTABLE,
    ANARCHY_PCIE_ERR_UNCORRECTABLE
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
};

#endif /* ANARCHY_PCIE_TYPES_H */
