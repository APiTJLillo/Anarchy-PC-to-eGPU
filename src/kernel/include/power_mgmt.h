#ifndef ANARCHY_POWER_MGMT_H
#define ANARCHY_POWER_MGMT_H

#include "forward.h"
#include "gpu_config.h"

/* Power states */
enum power_state {
    POWER_STATE_OFF = 0,
    POWER_STATE_LOW,
    POWER_STATE_NORMAL,
    POWER_STATE_BOOST,
    POWER_STATE_THROTTLE
};

/* Power profile configuration */
struct power_profile {
    unsigned int fan_speed;      /* Current fan speed percentage */
    unsigned int power_limit;    /* Current power limit in watts */
    bool dynamic_control;        /* Whether dynamic control is enabled */
    enum power_state state;      /* Current power state */
};

/* Power management functions */
int anarchy_power_init(struct anarchy_device *adev);
void anarchy_power_cleanup(struct anarchy_device *adev);
int anarchy_power_set_state(struct anarchy_device *adev, enum power_state state);
int anarchy_power_set_profile(struct anarchy_device *adev, const struct power_profile *profile);

#endif /* ANARCHY_POWER_MGMT_H */
