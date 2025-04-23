#include <linux/module.h>
#include <linux/pci.h>
#include <linux/thunderbolt.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include "include/anarchy_device.h"
#include "include/common.h"
#include "include/thunderbolt_internal.h"
#include "include/gpu_config.h"
#include "include/pcie_types.h"
#include "include/pcie_state.h"
#include "include/game_opt.h"
#include "include/game_compat.h"
#include "include/perf_monitor.h"
#include "include/command_proc.h"
#include "include/gpu_emu.h"
#include "include/usb4_config.h"
#include "include/thermal.h"
#include "include/thunderbolt_regs.h"
#include "include/thunderbolt_service.h"
#include "include/ring.h"

/* Forward declarations */
extern int power_limit;
extern int num_dma_channels;

/* Thunderbolt register access functions */
u32 tb_read32(struct anarchy_device *adev, u32 reg)
{
    if (!adev->mmio_base)
        return 0;
    return readl(adev->mmio_base + reg);
}
EXPORT_SYMBOL_GPL(tb_read32);

void tb_write32(struct anarchy_device *adev, u32 reg, u32 val)
{
    if (!adev->mmio_base)
        return;
    writel(val, adev->mmio_base + reg);
}
EXPORT_SYMBOL_GPL(tb_write32);

/* Initialize Thunderbolt interface */
int anarchy_tb_init(struct anarchy_device *adev)
{
    u32 val;
    int ret;

    /* Map Thunderbolt registers */
    adev->mmio_base = pci_iomap(adev->pdev, 2, 0);
    if (!adev->mmio_base) {
        dev_err(adev->dev, "Failed to map Thunderbolt registers\n");
        return -ENOMEM;
    }

    /* Enable Thunderbolt controller */
    val = tb_read32(adev, TB_CONTROL);
    val |= TB_CONTROL_ENABLE;
    tb_write32(adev, TB_CONTROL, val);

    /* Wait for controller to become ready */
    ret = readl_poll_timeout(adev->mmio_base + TB_STATUS,
                            val,
                            val & TB_STATUS_READY,
                            1000,
                            100000);
    if (ret) {
        dev_err(adev->dev, "Thunderbolt controller failed to become ready\n");
        goto err_unmap;
    }

    dev_info(adev->dev, "Thunderbolt interface initialized\n");
    return 0;

err_unmap:
    pci_iounmap(adev->pdev, adev->mmio_base);
    adev->mmio_base = NULL;
    return ret;
}
EXPORT_SYMBOL_GPL(anarchy_tb_init);

/* Cleanup Thunderbolt interface */
void anarchy_tb_fini(struct anarchy_device *adev)
{
    u32 val;

    if (!adev->mmio_base)
        return;

    /* Disable Thunderbolt controller */
    val = tb_read32(adev, TB_CONTROL);
    val &= ~TB_CONTROL_ENABLE;
    tb_write32(adev, TB_CONTROL, val);

    /* Unmap registers */
    pci_iounmap(adev->pdev, adev->mmio_base);
    adev->mmio_base = NULL;

    dev_info(adev->dev, "Thunderbolt interface shutdown complete\n");
}
EXPORT_SYMBOL_GPL(anarchy_tb_fini);

/* Initialize Thunderbolt subsystem */
int anarchy_thunderbolt_init(void)
{
    pr_info("Anarchy eGPU: Initializing Thunderbolt subsystem\n");
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_thunderbolt_init);

/* Cleanup Thunderbolt subsystem */
void anarchy_thunderbolt_cleanup(void)
{
    pr_info("Anarchy eGPU: Cleaning up Thunderbolt subsystem\n");
}
EXPORT_SYMBOL_GPL(anarchy_thunderbolt_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("Anarchy eGPU Thunderbolt Interface");
