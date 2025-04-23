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

void handle_device_connect(struct anarchy_device *adev)
{
    int ret;

    pr_info("Anarchy eGPU: Device connected\n");

    /* Initialize PCIe link */
    ret = anarchy_pcie_enable_link(adev);
    if (ret) {
        pr_err("Failed to enable PCIe link: %d\n", ret);
        return;
    }

    /* Wait for link to stabilize */
    msleep(100);

    /* Optimize PCIe settings */
    ret = anarchy_pcie_optimize_settings(adev);
    if (ret) {
        pr_err("Failed to optimize PCIe settings: %d\n", ret);
        goto disable_link;
    }

    /* Apply game-specific optimizations */
    ret = anarchy_optimize_for_game(adev, "default");
    if (ret) {
        pr_err("Failed to apply game optimizations: %d\n", ret);
        goto disable_link;
    }

    pr_info("Anarchy eGPU: Device initialization complete\n");
    return;

disable_link:
    anarchy_pcie_disable_link(adev);
}

void handle_device_disconnect(struct anarchy_device *adev)
{
    pr_info("Anarchy eGPU: Device disconnected\n");

    /* Disable PCIe link */
    anarchy_pcie_disable_link(adev);

    /* Reset device state */
    adev->pcie_state.state = ANARCHY_PCIE_STATE_DOWN;
    adev->pcie_state.enabled = false;
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
        dev_err(&adev->pdev->dev, "Failed to register hot-plug handler: %d\n", ret);
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
