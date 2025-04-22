#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/thunderbolt.h>
#include <linux/pci.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include "include/types.h"
#include "include/anarchy_device.h"
#include "include/service_pm.h"
#include "include/service_probe.h"
#include "include/common.h"
#include "include/gpu_power.h"
#include "thunderbolt.h"

/* Module parameters */
int power_limit __read_mostly = 175;
int num_dma_channels __read_mostly = 12;

module_param(power_limit, int, 0644);
MODULE_PARM_DESC(power_limit, "GPU power limit in watts (150-250)");

module_param(num_dma_channels, int, 0644);
MODULE_PARM_DESC(num_dma_channels, "Number of DMA channels (4-12)");

/* Forward declarations */
static void anarchy_service_shutdown(struct device *dev);

/* Service driver configuration */
static const struct tb_service_id anarchy_service_table[] = {
    {
        .match_flags = ANARCHY_SERVICE_MATCH_FLAGS,
        .protocol_key = ANARCHY_SERVICE_ID,
        .protocol_id = ANARCHY_SERVICE_ID,
        .protocol_version = ANARCHY_SERVICE_VERSION,
        .protocol_revision = 0,
        .driver_data = 0
    },
    { }  /* Terminator */
};

MODULE_DEVICE_TABLE(tbsvc, anarchy_service_table);

/* Service driver structure */
static struct tb_service_driver anarchy_service_driver = {
    .driver = {
        .name = "anarchy-egpu",
        .pm = &anarchy_service_pm,
        .shutdown = anarchy_service_shutdown,
    },
    .probe = anarchy_service_probe,
    .remove = anarchy_service_remove,
    .id_table = anarchy_service_table,
};

/* Shutdown callback for graceful device shutdown */
static void anarchy_service_shutdown(struct device *dev)
{
    struct tb_service *svc = tb_to_service(dev);
    struct anarchy_device *adev = tb_service_get_drvdata(svc);

    if (!adev)
        return;

    /* Disable performance monitoring */
    if (adev->perf_monitor.enabled) {
        cancel_delayed_work_sync(&adev->perf_monitor.update_work);
        adev->perf_monitor.enabled = false;
    }

    /* Power down the GPU */
    anarchy_gpu_power_down(adev);
}

static int __init anarchy_init(void)
{
    int ret;

    pr_info("Anarchy eGPU: Initializing mobile RTX 4090 driver\n");

    /* Validate module parameters */
    if (power_limit < 150 || power_limit > 250) {
        pr_err("Invalid power limit %d (must be between 150-250W)\n", power_limit);
        return -EINVAL;
    }

    if (num_dma_channels < 4 || num_dma_channels > 12) {
        pr_err("Invalid DMA channels %d (must be between 4-12)\n", num_dma_channels);
        return -EINVAL;
    }

    /* Initialize Thunderbolt subsystem */
    ret = tb_register_service_driver(&anarchy_service_driver);
    if (ret) {
        pr_err("Failed to register service driver: %d\n", ret);
        return ret;
    }

    pr_info("Anarchy eGPU: Mobile RTX 4090 driver initialized\n");
    pr_info("Power limit: %dW, DMA channels: %d\n", power_limit, num_dma_channels);
    pr_info("Waiting for USB4/TB4 connection from handheld device...\n");
    return 0;
}

static void __exit anarchy_exit(void)
{
    pr_info("Anarchy eGPU: Unloading driver\n");
    tb_unregister_service_driver(&anarchy_service_driver);
}

module_init(anarchy_init);
module_exit(anarchy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Development Team");
MODULE_DESCRIPTION("Mobile RTX 4090 eGPU Driver");
MODULE_VERSION("0.1.0");