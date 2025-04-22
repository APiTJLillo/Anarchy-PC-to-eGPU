#include <linux/module.h>
#include <linux/thunderbolt.h>
#include <linux/delay.h>
#include <linux/device.h>
#include "include/anarchy_device.h"
#include "include/hotplug.h"
#include "include/pcie_types.h"
#include "include/game_opt.h"
#include "include/perf_monitor.h"
#include "include/gpu_offsets.h"
#include "include/thunderbolt_service.h"

static void handle_device_connect(struct anarchy_device *adev)
{
    struct device *dev = &adev->dev;
    int ret;

    if (!adev || !dev)
        return;

    dev_info(dev, "Device connected\n");

    /* Initialize USB4 interface */
    ret = anarchy_usb4_init_device(adev);
    if (ret) {
        dev_err(dev, "Failed to initialize USB4 device: %d\n", ret);
        return;
    }

    /* Apply game optimizations */
    ret = anarchy_optimize_for_game(adev, "default");
    if (ret)
        dev_warn(dev, "Failed to apply game optimizations: %d\n", ret);        /* Enable PCIe link */
        ret = anarchy_pcie_enable_link(adev);
        if (ret) {
            dev_err(dev, "Failed to enable PCIe link: %d\n", ret);
            return;
        }

        /* Optimize PCIe settings for eGPU */
        ret = anarchy_pcie_optimize_settings(adev);
        if (ret)
            dev_warn(dev, "Failed to optimize PCIe settings: %d\n", ret);

    /* Start performance monitoring */
    schedule_delayed_work(&adev->perf_monitor.update_work, 0);
}

static void handle_device_disconnect(struct anarchy_device *adev)
{
    struct device *dev = &adev->dev;

    if (!adev || !dev)
        return;

    dev_info(dev, "Device disconnected\n");

    /* Stop performance monitoring */
    cancel_delayed_work_sync(&adev->perf_monitor.update_work);

    /* Disable PCIe link */
    anarchy_pcie_disable(&adev->pcie_state);
}

static void handle_hotplug_event(struct tb_service *service, void *data)
{
    struct anarchy_device *adev = tb_service_get_drvdata(service);
    bool connected = (bool)data;

    if (connected)
        handle_device_connect(adev);
    else
        handle_device_disconnect(adev);
}

int anarchy_hotplug_init(struct anarchy_device *adev)
{
    int ret;

    if (!adev || !adev->service)
        return -EINVAL;

    /* Store driver data in the service */
    tb_service_set_drvdata(adev->service, adev);

    /* Register hotplug handler */
    ret = tb_service_register_handler(adev->service, handle_hotplug_event);
    if (ret) {
        dev_err(&adev->dev, "Failed to register hot-plug handler: %d\n", ret);
        return ret;
    }

    return 0;
}

void anarchy_hotplug_exit(struct anarchy_device *adev)
{
    if (!adev || !adev->service)
        return;

    tb_service_unregister_handler(adev->service);
    tb_service_set_drvdata(adev->service, NULL);
}

EXPORT_SYMBOL_GPL(anarchy_hotplug_init);
EXPORT_SYMBOL_GPL(anarchy_hotplug_exit);
