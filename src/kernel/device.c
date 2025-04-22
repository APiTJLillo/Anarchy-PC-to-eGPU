#include <linux/module.h>
#include <linux/device.h>
#include "include/anarchy_device.h"
#include "include/common.h"

int anarchy_device_init(struct anarchy_device *adev)
{
    int ret;

    /* Initialize USB4/TB4 device mode */
    ret = anarchy_usb4_init_device(adev);
    if (ret)
        return ret;

    /* Initialize PCIe endpoint mode */
    adev->max_speed = 4;  /* PCIe Gen4 */
    adev->max_lanes = 8;  /* x8 over TB4 */
    adev->is_endpoint = true;

    /* Set up GPU configuration */
    ret = anarchy_gpu_init(adev);
    if (ret)
        goto err_usb4;

    /* Initialize DMA configuration */
    adev->dma_channels = 12;
    adev->ring_buffer_size = 512;
    adev->max_payload_size = 512;
    ret = anarchy_dma_init_gaming(adev);
    if (ret)
        goto err_gpu;

    /* Initialize game compatibility */
    ret = init_game_compatibility(adev);
    if (ret)
        goto err_dma;

    /* Initialize performance monitoring */
    ret = init_performance_monitoring(adev);
    if (ret)
        goto err_compat;

    /* Initialize bandwidth monitoring */
    ret = init_bandwidth_monitoring(adev);
    if (ret)
        goto err_perf;

    /* Initialize command processor */
    ret = init_command_processor(adev);
    if (ret)
        goto err_bw;

    /* Initialize thermal management */
    ret = anarchy_thermal_init(adev);
    if (ret)
        goto err_cmd;

    /* Initialize hot-plug detection */
    ret = anarchy_hotplug_init(adev);
    if (ret)
        goto err_thermal;

    dev_info(&adev->dev, "Device initialized successfully\n");
    return 0;

err_thermal:
    anarchy_thermal_exit(adev);
err_cmd:
    cleanup_command_processor(adev);
err_bw:
    cleanup_bandwidth_monitoring(adev);
err_perf:
    cleanup_performance_monitoring(adev);
err_compat:
    cleanup_game_compatibility(adev);
err_dma:
    anarchy_dma_cleanup(adev);
err_gpu:
    anarchy_gpu_cleanup(adev);
err_usb4:
    anarchy_usb4_exit_device(adev);
    return ret;
}
