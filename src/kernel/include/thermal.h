#ifndef ANARCHY_THERMAL_H
#define ANARCHY_THERMAL_H

#include "thermal_forward.h"

/* Thermal thresholds in millidegrees Celsius */
#define THERMAL_THRESHOLD_NORMAL   75000  /* 75°C */
#define THERMAL_THRESHOLD_WARNING  85000  /* 85°C */
#define THERMAL_THRESHOLD_CRITICAL 90000  /* 90°C */

/* Thermal monitoring interval in milliseconds */
#define THERMAL_MONITOR_INTERVAL 1000

/* Thermal monitoring functions */
int init_thermal_monitoring(struct anarchy_device *adev);
void cleanup_thermal_monitoring(struct anarchy_device *adev);

#endif /* ANARCHY_THERMAL_H */
