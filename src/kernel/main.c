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
#include "include/anarchy_driver.h"
#include "include/module_params.h"
#include "include/thunderbolt_service.h"

/* Module parameters */
int power_limit = 175;  /* Default power limit in watts */
int num_dma_channels = 8;  /* Default number of DMA channels */

module_param(power_limit, int, 0644);
MODULE_PARM_DESC(power_limit, "Power limit in watts (default: 175)");
module_param(num_dma_channels, int, 0644);
MODULE_PARM_DESC(num_dma_channels, "Number of DMA channels (default: 8)");

/* Forward declarations */
static void anarchy_service_shutdown(struct device *dev);
extern int anarchy_thunderbolt_init(void);
extern void anarchy_thunderbolt_cleanup(void);

/* Driver structure */
struct device_driver anarchy_driver = {
    .name = "anarchy",
    .owner = THIS_MODULE,
    .bus = &tb_bus_type,
    .shutdown = anarchy_service_shutdown,
};
EXPORT_SYMBOL_GPL(anarchy_driver);

#define ANARCHY_PROTOCOL_KEY 0x42  /* Replace with your actual protocol key */

/* Service ID table */
static const struct tb_service_id anarchy_service_table[] = {
    {
        .match_flags = TBSVC_MATCH_PROTOCOL_KEY,
        .protocol_key = ANARCHY_PROTOCOL_KEY,
        .protocol_id = 1,
        .protocol_version = 1,
    },
    { }
};

/* Service driver structure */
static struct tb_service_driver anarchy_service_driver = {
    .driver = {
        .name = "anarchy",
        .owner = THIS_MODULE,
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

/* Module initialization */
static int __init anarchy_init(void)
{
    int ret;

    pr_info("Anarchy eGPU: Initializing mobile RTX 4090 driver\n");

    /* Register the driver */
    ret = driver_register(&anarchy_driver);
    if (ret) {
        pr_err("Failed to register anarchy driver\n");
        return ret;
    }

    /* Initialize Thunderbolt subsystem */
    ret = anarchy_thunderbolt_init();
    if (ret) {
        pr_err("Failed to initialize Thunderbolt subsystem\n");
        driver_unregister(&anarchy_driver);
        return ret;
    }

    /* Register the service driver */
    ret = tb_register_service_driver(&anarchy_service_driver);
    if (ret) {
        pr_err("Failed to register anarchy service driver\n");
        anarchy_thunderbolt_cleanup();
        driver_unregister(&anarchy_driver);
        return ret;
    }

    pr_info("Anarchy eGPU: Mobile RTX 4090 driver initialized\n");
    pr_info("Power limit: %dW, DMA channels: %d\n", power_limit, num_dma_channels);
    pr_info("Waiting for USB4/TB4 connection from handheld device...\n");
    return 0;
}

/* Module cleanup */
static void __exit anarchy_exit(void)
{
    tb_unregister_service_driver(&anarchy_service_driver);
    anarchy_thunderbolt_cleanup();
    driver_unregister(&anarchy_driver);
    pr_info("Anarchy eGPU: Unloading driver\n");
}

module_init(anarchy_init);
module_exit(anarchy_exit);

/* Export module parameters */
EXPORT_SYMBOL_GPL(power_limit);
EXPORT_SYMBOL_GPL(num_dma_channels);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Development Team");
MODULE_DESCRIPTION("Mobile RTX 4090 eGPU Driver");
MODULE_VERSION("0.1.0");