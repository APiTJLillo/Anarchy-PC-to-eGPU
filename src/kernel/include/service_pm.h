#ifndef ANARCHY_SERVICE_PM_H
#define ANARCHY_SERVICE_PM_H

#include <linux/device.h>
#include "anarchy_device.h"

/* PM operations */
static int anarchy_service_suspend(struct device *dev)
{
    struct tb_service *svc = dev_get_drvdata(dev);
    struct anarchy_device *adev;

    if (!svc)
        return -ENODEV;

    adev = tb_service_get_drvdata(svc);
    if (!adev)
        return -ENODEV;

    /* Save device state and power down */
    return anarchy_device_suspend(adev);
}

static int anarchy_service_resume(struct device *dev)
{
    struct tb_service *svc = dev_get_drvdata(dev);
    struct anarchy_device *adev;

    if (!svc)
        return -ENODEV;

    adev = tb_service_get_drvdata(svc);
    if (!adev)
        return -ENODEV;

    /* Restore device state and power up */
    return anarchy_device_resume(adev);
}

/* PM ops structure - defined here to avoid static/extern conflicts */
const struct dev_pm_ops anarchy_service_pm = {
    .suspend = anarchy_service_suspend,
    .resume = anarchy_service_resume,
};

#endif /* ANARCHY_SERVICE_PM_H */
