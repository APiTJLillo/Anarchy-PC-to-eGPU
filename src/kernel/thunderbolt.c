#include <linux/module.h>
#include <linux/pci.h>
#include <linux/thunderbolt.h>
#include "include/anarchy_device.h"
#include "include/common.h"
#include "include/gpu_config.h"
#include "include/game_compat.h"
#include "include/perf_monitor.h"
#include "include/command_proc.h"
#include "include/gpu_emu.h"
#include "include/usb4_config.h"
#include "include/thermal.h"
#include "thunderbolt.h"

/* Performance monitoring function declarations */
extern int init_performance_monitoring(struct anarchy_device *adev);
extern void cleanup_performance_monitoring(struct anarchy_device *adev);

/* Forward declarations */
extern int power_limit;
extern int num_dma_channels;

/* Initialization sequence */
static int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id)
{
    struct anarchy_device *adev;
    struct device *dev = &svc->dev;
    struct gpu_emu_config emu_cfg;
    int ret;

    /* Allocate device structure */
    adev = devm_kzalloc(dev, sizeof(*adev), GFP_KERNEL);
    if (!adev)
        return -ENOMEM;

    /* Initialize base device */
    adev->dev.parent = dev;
    adev->service = svc;
    
    /* Set power configuration */
    adev->power_profile.power_limit = power_limit;
    adev->power_profile.fan_speed = FAN_SPEED_DEFAULT;
    adev->power_profile.dynamic_control = true;
    
    /* Set PCIe configuration */
    adev->pcie_state.link_speed = ANARCHY_PCIE_GEN4;
    adev->pcie_state.link_width = ANARCHY_PCIE_x8;
    adev->dma_channels = num_dma_channels;
    
    /* Configure memory mapping */
    adev->mmio_size = MMIO_SIZE;
    
    /* Configure GPU emulation */
    ret = anarchy_gpu_emu_init(adev);
    if (ret)
        goto err_emu;

    /* Initialize PCIe subsystem */
    ret = anarchy_pcie_init(adev);
    if (ret)
        goto err_pcie;
        
    /* Initialize game compatibility */
    ret = init_game_compatibility(adev);
    if (ret)
        goto err_compat;
        
    /* Initialize performance monitoring */
    ret = init_performance_monitoring(adev);
    if (ret)
        goto err_perf;
        
    /* Initialize command processor */
    ret = init_command_processor(adev);
    if (ret)
        goto err_cmd;
        
    /* Load default game profile */
    ret = anarchy_optimize_for_game(adev, "default");
    if (ret)
        goto err_profile;

    /* Store device pointer */
    tb_service_set_drvdata(svc, adev);
    
    dev_info(dev, "Anarchy eGPU: RTX 4090 Mobile initialized\n");
    return 0;

err_profile:
    cleanup_command_processor(adev);
err_cmd:
    cleanup_performance_monitoring(adev);
err_perf:
    cleanup_game_compatibility(adev);
err_compat:
    anarchy_pcie_exit(adev);
err_pcie:
    anarchy_gpu_emu_cleanup(adev);
err_emu:
    return ret;
}

static void anarchy_service_remove(struct tb_service *svc)
{
    struct anarchy_device *adev = tb_service_get_drvdata(svc);
    
    if (!adev)
        return;

    /* Stop DMA rings */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);
    
    /* Cleanup in reverse order */
    cleanup_command_processor(adev);
    cleanup_performance_monitoring(adev);
    cleanup_game_compatibility(adev);
    anarchy_pcie_exit(adev);
    anarchy_gpu_emu_cleanup(adev);
    
    tb_service_set_drvdata(svc, NULL);
}

/* Export symbols for other modules */
EXPORT_SYMBOL_GPL(anarchy_ring_start);
EXPORT_SYMBOL_GPL(anarchy_ring_stop);
EXPORT_SYMBOL_GPL(anarchy_ring_transfer);
EXPORT_SYMBOL_GPL(anarchy_ring_complete);
