#include <linux/module.h>
#include <linux/io.h>
#include "include/anarchy_device.h"
#include "include/pcie_mon.h"

u32 anarchy_pcie_get_link_errors(struct anarchy_device *adev)
{
    if (!adev)
        return 0;
    return readl(adev->mmio_base + PCIE_ERROR_OFFSET);
}

void anarchy_pcie_clear_error_counters(struct anarchy_device *adev)
{
    if (!adev)
        return;
    writel(0, adev->mmio_base + PCIE_ERROR_OFFSET);
}

EXPORT_SYMBOL_GPL(anarchy_pcie_get_link_errors);
EXPORT_SYMBOL_GPL(anarchy_pcie_clear_error_counters);
