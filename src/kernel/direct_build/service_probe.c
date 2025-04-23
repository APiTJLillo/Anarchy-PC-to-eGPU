#include <linux/module.h>
#include <linux/device.h>
#include <linux/thunderbolt.h>
#include "include/service_probe.h"
#include "include/anarchy_device.h"
#include "include/common.h"
#include "include/gpu_config.h"
#include "include/gpu_emu.h"
#include "include/pcie_state.h"
#include "include/game_compat.h"
#include "include/perf_monitor.h"
#include "include/command_proc.h"
#include "include/usb4_config.h"
#include "include/thermal.h"
#include "include/pcie.h"
#include "include/anarchy_driver.h"
#include "include/module_params.h"

/* Service probe callback */
int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id)
{
    struct device *dev = &svc->dev;
    struct anarchy_device *adev;
    int ret;

    /* Allocate device structure */
    adev = kzalloc(sizeof(*adev), GFP_KERNEL);
    if (!adev)
        return -ENOMEM;

    /* Initialize device fields */
    adev->service = svc;
    adev->state = ANARCHY_DEVICE_STATE_INITIALIZING;
    adev->flags = 0;
    mutex_init(&adev->lock);
    atomic_set(&adev->ref_count, 1);

    /* Allocate and initialize device */
    adev->dev = kzalloc(sizeof(struct device), GFP_KERNEL);
    if (!adev->dev) {
        ret = -ENOMEM;
        goto err_free_adev;
    }

    device_initialize(adev->dev);
    adev->dev->parent = dev;
    adev->dev->driver = &anarchy_driver;
    dev_set_name(adev->dev, "anarchy-%s", dev_name(dev));

    /* Store device data */
    dev_set_drvdata(dev, adev);

    /* Initialize device subsystems */
    ret = anarchy_device_init(adev);
    if (ret)
        goto err_free_dev;

    /* Register device */
    ret = device_add(adev->dev);
    if (ret)
        goto err_cleanup;

    return 0;

err_cleanup:
    anarchy_device_exit(adev);
err_free_dev:
    kfree(adev->dev);
err_free_adev:
    kfree(adev);
    return ret;
}
EXPORT_SYMBOL_GPL(anarchy_service_probe);

/* Service remove callback */
void anarchy_service_remove(struct tb_service *svc)
{
    struct device *dev = &svc->dev;
    struct anarchy_device *adev = dev_get_drvdata(dev);

    if (!adev)
        return;

    /* Unregister device */
    device_del(adev->dev);

    /* Cleanup device */
    anarchy_device_exit(adev);

    /* Free device structures */
    kfree(adev->dev);
    kfree(adev);
}
EXPORT_SYMBOL_GPL(anarchy_service_remove);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("Anarchy eGPU Service Probe"); 