#ifndef ANARCHY_PCIE_H
#define ANARCHY_PCIE_H

#include "types.h"

/* PCIe Link Configuration */
int pcie_set_link_speed(struct anarchy_device *adev, enum anarchy_pcie_speed speed);
int pcie_set_link_width(struct anarchy_device *adev, enum anarchy_pcie_width width);
int pcie_get_current_link_speed(struct anarchy_device *adev);
int pcie_get_current_link_width(struct anarchy_device *adev);

#endif /* ANARCHY_PCIE_H */
