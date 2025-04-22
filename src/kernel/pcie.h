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

#endif /* ANARCHY_PCIE_H */