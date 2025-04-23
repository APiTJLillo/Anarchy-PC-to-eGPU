#ifndef ANARCHY_PCIE_FORWARD_H
#define ANARCHY_PCIE_FORWARD_H

/* Forward declarations */
struct anarchy_device;
struct anarchy_pcie_state;
struct anarchy_pcie_recovery;

/* PCIe link states */
enum anarchy_pcie_link_state {
    ANARCHY_PCIE_STATE_UNKNOWN = 0,
    ANARCHY_PCIE_STATE_DOWN,
    ANARCHY_PCIE_STATE_TRAINING,
    ANARCHY_PCIE_STATE_ACTIVE,
    ANARCHY_PCIE_STATE_ERROR,
    ANARCHY_PCIE_STATE_NORMAL,
    ANARCHY_PCIE_STATE_RECOVERY,
    ANARCHY_PCIE_STATE_INIT,
    ANARCHY_PCIE_STATE_LINK_DOWN
};

/* PCIe ASPM policies */
#define ANARCHY_ASPM_DISABLED      0x0  /* ASPM disabled */
#define ANARCHY_ASPM_L0s          0x1  /* L0s enabled */
#define ANARCHY_ASPM_L1           0x2  /* L1 enabled */
#define ANARCHY_ASPM_L0sL1        0x3  /* L0s and L1 enabled */
#define ANARCHY_ASPM_PERFORMANCE  0x4  /* Performance optimized policy */

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

/* PCIe link widths */
enum anarchy_pcie_width {
    ANARCHY_PCIE_WIDTH_UNKNOWN = 0,
    ANARCHY_PCIE_WIDTH_x1 = 1,
    ANARCHY_PCIE_WIDTH_x2 = 2,
    ANARCHY_PCIE_WIDTH_x4 = 4,
    ANARCHY_PCIE_WIDTH_x8 = 8,
    ANARCHY_PCIE_WIDTH_x16 = 16
};

/* PCIe error types */
enum anarchy_pcie_error_type {
    ANARCHY_PCIE_ERR_NONE = 0,
    ANARCHY_PCIE_ERR_TRAINING,
    ANARCHY_PCIE_ERR_LINK_DOWN,
    ANARCHY_PCIE_ERR_TIMEOUT,
    ANARCHY_PCIE_ERR_CORRECTABLE,
    ANARCHY_PCIE_ERR_UNCORRECTABLE,
    ANARCHY_PCIE_ERR_SPEED_DOWNGRADE,
    ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE
};

/* PCIe constants */
#define ANARCHY_PCIE_MAX_RETRIES 3

#endif /* ANARCHY_PCIE_FORWARD_H */