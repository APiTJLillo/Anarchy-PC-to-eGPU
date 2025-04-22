#ifndef ANARCHY_PCIE_TYPES_H
#define ANARCHY_PCIE_TYPES_H

#include <linux/types.h>

/* PCIe link speed definitions */
enum anarchy_pcie_speed {
    ANARCHY_PCIE_GEN1 = 1,    /* 2.5 GT/s */
    ANARCHY_PCIE_GEN2 = 2,    /* 5.0 GT/s */
    ANARCHY_PCIE_GEN3 = 3,    /* 8.0 GT/s */
    ANARCHY_PCIE_GEN4 = 4,    /* 16.0 GT/s */
    ANARCHY_PCIE_GEN5 = 5     /* 32.0 GT/s */
};

/* PCIe link width definitions */
enum anarchy_pcie_width {
    ANARCHY_PCIE_x1 = 1,
    ANARCHY_PCIE_x2 = 2,
    ANARCHY_PCIE_x4 = 4,
    ANARCHY_PCIE_x8 = 8,
    ANARCHY_PCIE_x16 = 16
};

/* PCIe error codes */
enum anarchy_pcie_error {
    ANARCHY_PCIE_ERR_NONE = 0,
    ANARCHY_PCIE_ERR_LINK_DOWN = -1,
    ANARCHY_PCIE_ERR_TRAINING = -2,
    ANARCHY_PCIE_ERR_SPEED = -3,
    ANARCHY_PCIE_ERR_WIDTH = -4,
    ANARCHY_PCIE_ERR_TIMEOUT = -5
};

/* PCIe configuration space access modes */
enum anarchy_pcie_access {
    ANARCHY_PCIE_ACCESS_MMIO = 0,
    ANARCHY_PCIE_ACCESS_CONFIG = 1
};

/* PCIe capabilities */
struct anarchy_pcie_caps {
    enum anarchy_pcie_speed max_speed;
    enum anarchy_pcie_width max_width;
    bool ats_support;
    bool sriov_support;
    bool aspm_support;
    u32 max_payload_size;
    u32 max_read_request_size;
};

#endif /* ANARCHY_PCIE_TYPES_H */
