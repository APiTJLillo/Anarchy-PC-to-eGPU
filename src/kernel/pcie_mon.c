#include <linux/module.h>
#include <linux/io.h>
#include "include/anarchy_device.h"
#include "include/pcie_mon.h"

u32 anarchy_pcie_get_bandwidth_usage(struct anarchy_device *adev)
{
    u32 bw_used, bw_total;

    if (!adev)
        return 0;

    /* Read current bandwidth usage and total capacity */
    bw_used = readl(adev->mmio_base + PCIE_BANDWIDTH_OFFSET);
    bw_total = readl(adev->mmio_base + PCIE_UTIL_OFFSET);

    /* Return utilization as a percentage */
    if (bw_total == 0)
        return 0;
    return (bw_used * 100) / bw_total;
}

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

EXPORT_SYMBOL_GPL(anarchy_pcie_get_bandwidth_usage);
EXPORT_SYMBOL_GPL(anarchy_pcie_get_link_errors);
EXPORT_SYMBOL_GPL(anarchy_pcie_clear_error_counters);
