#ifndef ANARCHY_PCIE_H
#define ANARCHY_PCIE_H

#include "pcie_forward.h"
#include "pcie_types.h"

/* PCIe speed definitions */
#define PCIE_GEN4_SPEED   16  /* Gen4 speed in GT/s */
#include "common.h"  /* Use common definitions for PCIe configuration */

/* PCIe initialization and cleanup */
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);

/* PCIe link control */
void anarchy_pcie_disable_link(struct anarchy_device *adev);
int anarchy_pcie_train_link(struct anarchy_device *adev);
int anarchy_pcie_retrain_link(struct anarchy_device *adev);

/* PCIe error handling */
void anarchy_pcie_handle_error(struct anarchy_device *adev,
                             enum anarchy_pcie_error_type error);

#endif /* ANARCHY_PCIE_H */
