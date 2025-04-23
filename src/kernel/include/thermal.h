#ifndef ANARCHY_THERMAL_H
#define ANARCHY_THERMAL_H

#include "types.h"
#include "thermal_forward.h"
#include "gpu_config.h"  /* For fan speed thresholds */

/* Thermal threshold constants */
#define THERMAL_THRESHOLD_NORMAL   70000  /* 70°C */
#define THERMAL_THRESHOLD_WARNING  80000  /* 80°C */
#define THERMAL_THRESHOLD_CRITICAL 87000  /* 87°C */

/* Update interval */
#define THERMAL_MONITOR_INTERVAL 1000  /* 1 second */

/* Thermal monitoring functions */
int init_thermal_monitoring(struct anarchy_device *adev);
void cleanup_thermal_monitoring(struct anarchy_device *adev);

#endif /* ANARCHY_THERMAL_H */
