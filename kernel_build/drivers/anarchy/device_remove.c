#include <linux/module.h>
#include <linux/delay.h>
#include "include/common.h"
#include "include/anarchy_device.h"

static void cleanup_gpu_resources(struct anarchy_device *adev)
{
    /* Wait for pending operations to complete */
    flush_workqueue(adev->wq);
    
    /* Disable performance monitoring */
    if (adev->perf_monitor) {
        cancel_delayed_work_sync(&adev->perf_monitor->work);
    }

    /* Save current game settings if needed */
    if (adev->compat_layer && adev->compat_layer->current_game) {
        dev_info(&adev->dev, "Saving settings for game: %s\n",
                 adev->compat_layer->current_game);
    }

    /* Disable GPU features gracefully */
    if (adev->gpu_ops && adev->gpu_ops->set_power_limit) {
        adev->gpu_ops->set_power_limit(adev, MIN_POWER_LIMIT);
    }

    /* Wait for GPU to idle */
    msleep(100);
}

void anarchy_device_remove(struct anarchy_device *adev)
{
    dev_info(&adev->dev, "Removing eGPU device\n");

    /* Clean up GPU resources first */
    cleanup_gpu_resources(adev);

    /* Disable USB4/TB4 device mode */
    anarchy_usb4_exit_device(adev);

    /* Clean up other components */
    anarchy_thermal_exit(adev);
    cleanup_command_processor(adev);
    cleanup_bandwidth_monitoring(adev);
    cleanup_performance_monitoring(adev);
    cleanup_game_compatibility(adev);
    anarchy_dma_cleanup(adev);
    anarchy_gpu_cleanup(adev);

    dev_info(&adev->dev, "eGPU device removed successfully\n");
}
