#include <linux/module.h>
#include <linux/thunderbolt.h>
#include "include/usb4_config.h"

/* Hot-plug event flags */
#define HP_EVENT_CONNECTED    BIT(0)
#define HP_EVENT_AUTHORIZED   BIT(1)
#define HP_EVENT_LINK_READY   BIT(2)
#define HP_EVENT_ERROR        BIT(3)

static void handle_hotplug_event(struct anarchy_device *adev, unsigned int flags)
{
    struct device *dev = &adev->dev;

    if (flags & HP_EVENT_ERROR) {
        dev_err(dev, "Hot-plug error detected\n");
        return;
    }

    if (flags & HP_EVENT_CONNECTED) {
        dev_info(dev, "USB4/TB4 host device connected\n");
        
        /* Initialize eGPU device mode */
        if (anarchy_usb4_init_device(adev))
            return;

        /* Apply gaming optimizations */
        anarchy_optimize_for_game(adev, "default");
    }

    if (flags & HP_EVENT_AUTHORIZED) {
        dev_info(dev, "Connection authorized by host\n");
        
        /* Enable PCIe link */
        if (anarchy_pcie_enable(&adev->pcie))
            dev_err(dev, "Failed to enable PCIe link\n");
    }

    if (flags & HP_EVENT_LINK_READY) {
        dev_info(dev, "Link training complete - eGPU ready\n");
        
        /* Start performance monitoring */
        queue_delayed_work(adev->perf_monitor->wq, 
                         &adev->perf_monitor->work, 0);
    }
}

int anarchy_hotplug_init(struct anarchy_device *adev)
{
    int ret;

    /* Register hot-plug handler */
    ret = tb_register_device_handler(adev->tb_switch, handle_hotplug_event);
    if (ret) {
        dev_err(&adev->dev, "Failed to register hot-plug handler: %d\n", ret);
        return ret;
    }

    return 0;
}
