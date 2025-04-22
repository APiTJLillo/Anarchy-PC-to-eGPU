#ifndef ANARCHY_PCIE_H
#define ANARCHY_PCIE_H

#include "anarchy_device.h"

/* PCIe speed definitions */
#define PCIE_GEN4_SPEED   16  /* Gen4 speed in GT/s */
#define PCIE_MAX_LANES    16  /* Maximum number of PCIe lanes */
#define RING_BUFFER_SIZE  (32 * 1024)  /* 32KB ring buffer */

/* PCIe initialization and cleanup */
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_cleanup(struct anarchy_device *adev);

#endif /* ANARCHY_PCIE_H */
