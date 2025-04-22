#ifndef ANARCHY_PCIE_STATE_H
#define ANARCHY_PCIE_STATE_H

#include <linux/types.h>
#include "forward.h"
#include "pcie_types.h"

/* PCIe state management */
int anarchy_pcie_init_state(struct anarchy_device *adev);
void anarchy_pcie_cleanup_state(struct anarchy_device *adev);

/* PCIe link control */
int anarchy_pcie_enable_link(struct anarchy_device *adev);
void anarchy_pcie_disable_link(struct anarchy_device *adev);
int anarchy_pcie_retrain_link(struct anarchy_device *adev);

/* PCIe error handling */
void anarchy_pcie_handle_error(struct anarchy_device *adev, enum anarchy_pcie_error_type error);
int anarchy_pcie_get_error_stats(struct anarchy_device *adev, u32 *error_count);
void anarchy_pcie_clear_errors(struct anarchy_device *adev);

/* PCIe configuration */
int anarchy_pcie_set_speed(struct anarchy_device *adev, enum anarchy_pcie_speed speed);
int anarchy_pcie_set_width(struct anarchy_device *adev, u32 width);
int anarchy_pcie_optimize_settings(struct anarchy_device *adev);

#endif /* ANARCHY_PCIE_STATE_H */
