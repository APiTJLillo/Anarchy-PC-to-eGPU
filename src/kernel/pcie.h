#ifndef ANARCHY_PCIE_H
#define ANARCHY_PCIE_H

#include <linux/pci.h>
#include "include/anarchy_device.h"
#include "include/pcie_types.h"

/* PCIe link state checks */
bool pcie_link_is_up(struct pci_dev *pdev);
void pcie_reset_link(struct pci_dev *pdev);

/* Thunderbolt service functions */
static inline bool tb_service_reset(struct tb_service *svc)
{
    struct device *dev;
    if (!svc)
        return false;
    dev = &svc->dev;
    if (dev && dev->driver && dev->driver->shutdown) {
        dev->driver->shutdown(dev);
        return true;
    }
    return false;
}

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