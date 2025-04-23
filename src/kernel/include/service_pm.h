#ifndef ANARCHY_SERVICE_PM_H
#define ANARCHY_SERVICE_PM_H

#include <linux/device.h>
#include "anarchy_device.h"

/* Function declarations */
int anarchy_service_suspend(struct device *dev);
int anarchy_service_resume(struct device *dev);

/* PM ops structure - declared here, defined in service_pm.c */
extern const struct dev_pm_ops anarchy_service_pm;

#endif /* ANARCHY_SERVICE_PM_H */
