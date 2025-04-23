#ifndef ANARCHY_BANDWIDTH_H
#define ANARCHY_BANDWIDTH_H

#include <linux/types.h>

struct anarchy_device;

/* Function to get PCIe bandwidth usage */
u64 anarchy_pcie_get_bandwidth_usage(struct anarchy_device *adev);

#endif /* ANARCHY_BANDWIDTH_H */
