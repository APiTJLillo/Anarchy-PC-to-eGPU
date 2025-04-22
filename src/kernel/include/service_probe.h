#ifndef ANARCHY_SERVICE_PROBE_H
#define ANARCHY_SERVICE_PROBE_H

#include <linux/thunderbolt.h>
#include "anarchy_device.h"
#include "common.h"
#include "gpu_config.h"
#include "gpu_emu.h"
#include "pcie_state.h"
#include "game_compat.h"
#include "perf_monitor.h"
#include "command_proc.h"
#include "usb4_config.h"
#include "thermal.h"
#include "pcie.h"

/* Declare module parameters */
extern int power_limit;
extern int num_dma_channels;

/* Service probe callback */
static int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id)
{
    struct anarchy_device *adev;
    struct device *dev = &svc->dev;
    int ret;

    /* Allocate device structure */
    adev = devm_kzalloc(dev, sizeof(*adev), GFP_KERNEL);
    if (!adev)
        return -ENOMEM;

    /* Basic device setup */
    adev->dev.parent = dev;
    adev->service = svc;
    
    /* Initialize power profile */
    adev->current_profile.fan_speed = P16_FAN_MIN_SPEED;
    adev->current_profile.power_limit = power_limit;
    adev->current_profile.dynamic_control = true;
    
    /* Initialize device capabilities */
    adev->max_speed = PCIE_GEN4_SPEED;
    adev->max_lanes = PCIE_MAX_LANES;
    adev->dma_channels = num_dma_channels;
    adev->ring_buffer_size = RING_BUFFER_SIZE;

    /* Initialize PCIe subsystem */
    ret = anarchy_pcie_init(adev);
    if (ret)
        return ret;

    /* Initialize GPU emulation */
    ret = anarchy_gpu_emu_init(adev);
    if (ret)
        goto err_pcie;

    /* Initialize game compatibility */
    ret = init_game_compatibility(adev);
    if (ret)
        goto err_emu;

    /* Initialize performance monitoring */
    ret = init_performance_monitoring(adev);
    if (ret)
        goto err_compat;

    /* Initialize command processor */
    ret = init_command_processor(adev);
    if (ret)
        goto err_perf;

    /* Store device pointer */
    tb_service_set_drvdata(svc, adev);

    dev_info(dev, "Anarchy eGPU: RTX 4090 Mobile initialized\n");
    dev_info(dev, "Power limit: %dW, DMA channels: %d\n", 
             power_limit, num_dma_channels);
    return 0;

err_perf:
    cleanup_performance_monitoring(adev);
err_compat:
    cleanup_game_compatibility(adev);
err_emu:
    anarchy_gpu_emu_cleanup(adev);
err_pcie:
    anarchy_pcie_exit(adev);
    return ret;
}

/* Service remove callback */
static void anarchy_service_remove(struct tb_service *svc)
{
    struct anarchy_device *adev = tb_service_get_drvdata(svc);
    struct device *dev = &svc->dev;
    
    if (!adev)
        return;

    dev_info(dev, "Removing RTX 4090 Mobile device\n");

    /* Stop all operations */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);

    /* Cleanup in reverse order */
    cleanup_command_processor(adev);
    cleanup_performance_monitoring(adev);
    cleanup_game_compatibility(adev);
    anarchy_gpu_emu_cleanup(adev);
    anarchy_pcie_exit(adev);

    tb_service_set_drvdata(svc, NULL);
    dev_info(dev, "RTX 4090 Mobile device removed\n");
}

#endif /* ANARCHY_SERVICE_PROBE_H */
