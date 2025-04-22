#ifndef ANARCHY_POWER_MGMT_H
#define ANARCHY_POWER_MGMT_H

#include <linux/types.h>
#include "forward.h"

/* Power profile structure */
#ifndef ANARCHY_POWER_PROFILE_DEFINED
#define ANARCHY_POWER_PROFILE_DEFINED

struct power_profile {
    unsigned int fan_speed;     /* Current fan speed percentage */
    unsigned int power_limit;   /* Current power limit in watts */
    bool dynamic_control;       /* Whether dynamic control is enabled */
};

#endif /* ANARCHY_POWER_PROFILE_DEFINED */

/* Power management functions */
int anarchy_power_init(struct anarchy_device *adev);
void anarchy_power_exit(struct anarchy_device *adev);
int anarchy_power_set_profile(struct anarchy_device *adev, struct power_profile *profile);
int anarchy_power_get_profile(struct anarchy_device *adev, struct power_profile *profile);
int anarchy_power_set_fan_speed(struct anarchy_device *adev, unsigned int speed);
int anarchy_power_set_power_limit(struct anarchy_device *adev, unsigned int limit);

#endif /* ANARCHY_POWER_MGM_H */
