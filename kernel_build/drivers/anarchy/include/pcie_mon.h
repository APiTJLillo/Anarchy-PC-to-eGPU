#ifndef ANARCHY_PCIE_MON_H
#define ANARCHY_PCIE_MON_H

#include <linux/types.h>
#include "pcie_forward.h"
#include "bandwidth.h"

/* PCIe bandwidth monitoring registers */
#define PCIE_BANDWIDTH_OFFSET     0x5000
#define PCIE_UTIL_OFFSET         0x5004

/* PCIe monitoring registers */
#define PCIE_ERROR_OFFSET        0x5008

/* PCIe monitoring functions */
u32 anarchy_pcie_get_link_errors(struct anarchy_device *adev);
void anarchy_pcie_clear_error_counters(struct anarchy_device *adev);

#endif /* ANARCHY_PCIE_MON_H */
