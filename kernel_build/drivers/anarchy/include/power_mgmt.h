#ifndef ANARCHY_POWER_MGMT_H
#define ANARCHY_POWER_MGMT_H

#include "types.h"

/* Power management constants */
#define GPU_POWER_LIMIT_MIN     50   /* Minimum power limit in watts */
#define GPU_POWER_LIMIT_MAX     300  /* Maximum power limit in watts */
#define GPU_POWER_LIMIT_DEFAULT 175  /* Default power limit in watts */

/* Power profile structure */
struct power_profile {
    u32 power_limit;        /* Power limit in watts */
    u32 fan_speed;         /* Fan speed percentage (0-100) */
    bool dynamic_control;   /* Whether to use dynamic fan control */
};

/* Power management functions */
int anarchy_power_init(struct anarchy_device *adev);
void anarchy_power_exit(struct anarchy_device *adev);
int anarchy_power_set_profile(struct anarchy_device *adev, struct power_profile *profile);
int anarchy_power_get_profile(struct anarchy_device *adev, struct power_profile *profile);
int anarchy_power_set_fan_speed(struct anarchy_device *adev, unsigned int speed);
int anarchy_power_set_power_limit(struct anarchy_device *adev, unsigned int limit);
int anarchy_power_get_power_limit(struct anarchy_device *adev, u32 *limit);

/* Power management functions */
int anarchy_power_init(struct anarchy_device *adev);
void anarchy_power_fini(struct anarchy_device *adev);
int anarchy_power_set_power_limit(struct anarchy_device *adev, u32 limit);
int anarchy_power_get_power_limit(struct anarchy_device *adev, u32 *limit);

#endif /* ANARCHY_POWER_MGMT_H */
