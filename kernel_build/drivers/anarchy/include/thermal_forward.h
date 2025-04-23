#ifndef ANARCHY_THERMAL_FORWARD_H
#define ANARCHY_THERMAL_FORWARD_H

#include <linux/types.h>
#include <linux/workqueue.h>

/* Forward declarations */
struct anarchy_device;

/* Thermal callback function type */
typedef void (*thermal_callback_t)(struct anarchy_device *adev);

/* Thermal profile structure */
struct thermal_profile {
    struct workqueue_struct *wq;
    struct delayed_work update_work;
    spinlock_t lock;
    bool monitoring_enabled;
    bool throttling;
    int current_temp;
    int target_fan_speed;
    unsigned long last_update;
    int max_temp;
    int warning_threshold;
    int critical_threshold;
    thermal_callback_t warning_callback;
    thermal_callback_t critical_callback;
};

#endif /* ANARCHY_THERMAL_FORWARD_H */ 