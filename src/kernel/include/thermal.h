#ifndef ANARCHY_THERMAL_H
#define ANARCHY_THERMAL_H

#include <linux/types.h>
#include <linux/thermal.h>
#include "gpu_config.h"
#include "anarchy_device.h"

/* Thermal profile structure */
struct power_profile {
    unsigned int fan_speed;      /* Current fan speed percentage */
    unsigned int power_limit;    /* Current power limit in watts */
    bool dynamic_control;        /* Whether dynamic control is enabled */
};

/* Thermal management functions */
void adjust_thermal_profile(struct anarchy_device *adev, int temp);
int anarchy_gpu_set_fan_speed(struct anarchy_device *adev, unsigned int speed);
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, unsigned int limit);

/* Thermal constants */
#ifndef GPU_THERMAL_TARGET
#define GPU_THERMAL_TARGET 75  /* Target temperature in Celsius */
#endif

#ifndef GPU_THERMAL_CRIT
#define GPU_THERMAL_CRIT  85  /* Critical temperature in Celsius */
#endif

#ifndef FAN_SPEED_MIN
#define FAN_SPEED_MIN    20  /* Minimum fan speed percentage */
#endif

#ifndef FAN_SPEED_MAX
#define FAN_SPEED_MAX   100  /* Maximum fan speed percentage */
#endif

/* Thermal constants */
#define GPU_THERMAL_TARGET 75  /* Target temperature in Celsius */
#define GPU_THERMAL_CRIT  85  /* Critical temperature in Celsius */
#define FAN_SPEED_MIN    20  /* Minimum fan speed percentage */
#define FAN_SPEED_MAX   100  /* Maximum fan speed percentage */

/* Thermal profile structure */
struct thermal_profile {
    int temp_target;     /* Target temperature */
    int temp_max;        /* Maximum temperature */
    int fan_min;         /* Minimum fan speed */
    int fan_max;         /* Maximum fan speed */
    int power_limit;     /* Power limit in watts */
    bool dynamic_fan;    /* Dynamic fan control enabled */
};

#endif /* ANARCHY_THERMAL_H */
