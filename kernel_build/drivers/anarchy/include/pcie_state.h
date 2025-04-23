#ifndef __ANARCHY_PCIE_STATE_H__
#define __ANARCHY_PCIE_STATE_H__

#include <linux/types.h>
#include "pcie_forward.h"
#include "pcie_types.h"

struct anarchy_device;

/* PCIe initialization and cleanup */
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);

/* PCIe link management */
int anarchy_pcie_train_link(struct anarchy_device *adev);
int anarchy_pcie_enable_link(struct anarchy_device *adev);
void anarchy_pcie_disable_link(struct anarchy_device *adev);
void anarchy_pcie_disable(struct anarchy_device *adev);

/* PCIe state management */
int anarchy_pcie_init_state(struct anarchy_device *adev);
void anarchy_pcie_cleanup_state(struct anarchy_device *adev);
void anarchy_pcie_handle_error(struct anarchy_device *adev, enum anarchy_pcie_error_type error);

/* PCIe configuration */
int anarchy_pcie_optimize_settings(struct anarchy_device *adev);
bool pcie_link_is_up(struct pci_dev *pdev);

/* PCIe error handling */
int anarchy_pcie_get_error_stats(struct anarchy_device *adev, u32 *error_count);
void anarchy_pcie_clear_errors(struct anarchy_device *adev);

/* PCIe configuration */
int anarchy_pcie_set_speed(struct anarchy_device *adev, enum anarchy_pcie_speed speed);
int anarchy_pcie_set_width(struct anarchy_device *adev, enum anarchy_pcie_width width);

#endif /* __ANARCHY_PCIE_STATE_H__ */
