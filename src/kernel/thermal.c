#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/workqueue.h>
#include "include/common.h"
#include "include/power_mgmt.h"
#include "include/perf_monitor.h"

/* Thermal thresholds */
#define P16_FAN_MIN_SPEED    30  /* 30% minimum */
#define P16_FAN_MAX_SPEED    100 /* 100% maximum */
#define P16_TEMP_TARGET      75  /* 75°C target */
#define P16_TEMP_CRITICAL    87  /* 87°C critical */
#define P16_TEMP_WARNING     80  /* 80°C warning */
#define THERMAL_UPDATE_INTERVAL_MS 1000

/* Fan speed steps (percentage) */
static const int fan_speed_steps[] = {
    30,  /* Silent - minimum speed */
    40,  /* Very low */
    50,  /* Low */
    60,  /* Medium */
    70,  /* Medium high */
    80,  /* High */
    90,  /* Very high */
    100  /* Maximum */
};

struct thermal_profile {
    struct workqueue_struct *wq;
    struct delayed_work update_work;
    spinlock_t lock;
    bool throttling;
    int current_temp;
    int target_fan_speed;
    unsigned long last_update;
};

static void adjust_thermal_profile(struct anarchy_device *adev, int temp)
{
    struct power_profile profile;
    int i, target_fan_speed;
    
    /* Get current power profile */
    anarchy_power_get_profile(adev, &profile);

    /* Adjust fan speed based on temperature */
    if (temp >= P16_TEMP_CRITICAL) {
        /* Emergency cooling */
        target_fan_speed = P16_FAN_MAX_SPEED;
        profile.power_limit = 150; /* Drop to base TGP */
        profile.dynamic_control = false;
    } else if (temp >= P16_TEMP_WARNING) {
        /* Aggressive cooling */
        target_fan_speed = fan_speed_steps[6]; /* Very high */
        profile.power_limit = 175; /* Reduced power limit */
    } else if (temp >= P16_TEMP_TARGET) {
        /* Gradual fan speed increase */
        for (i = 0; i < ARRAY_SIZE(fan_speed_steps); i++) {
            if (temp < (P16_TEMP_TARGET + (i * 2))) {
                target_fan_speed = fan_speed_steps[i];
                break;
            }
        }
    } else {
        /* Normal operation */
        target_fan_speed = fan_speed_steps[2]; /* Low speed */
        profile.dynamic_control = true;
    }

    /* Update fan speed if changed */
    if (profile.fan_speed != target_fan_speed) {
        profile.fan_speed = target_fan_speed;
        anarchy_power_set_profile(adev, &profile);
    }
}

static void thermal_update_work(struct work_struct *work)
{
    struct thermal_profile *thermal = container_of(to_delayed_work(work),
                                                 struct thermal_profile,
                                                 update_work);
    struct anarchy_device *adev = container_of(thermal, struct anarchy_device,
                                             thermal);
    struct perf_state perf_state;
    unsigned long flags;
    
    /* Get current performance state */
    anarchy_perf_get_state(adev, &perf_state);
    
    spin_lock_irqsave(&thermal->lock, flags);
    thermal->current_temp = perf_state.temperature;
    thermal->last_update = jiffies;
    
    /* Adjust thermal profile based on current temperature */
    adjust_thermal_profile(adev, thermal->current_temp);
    
    spin_unlock_irqrestore(&thermal->lock, flags);
    
    /* Schedule next update */
    schedule_delayed_work(&thermal->update_work,
                         msecs_to_jiffies(THERMAL_UPDATE_INTERVAL_MS));
}

int init_thermal_monitoring(struct anarchy_device *adev)
{
    struct thermal_profile *thermal = &adev->thermal;
    
    /* Initialize thermal monitoring state */
    thermal->throttling = false;
    thermal->current_temp = 0;
    thermal->target_fan_speed = fan_speed_steps[2];
    
    /* Initialize synchronization */
    spin_lock_init(&thermal->lock);
    
    /* Initialize work queue */
    INIT_DELAYED_WORK(&thermal->update_work, thermal_update_work);
    
    /* Start monitoring */
    schedule_delayed_work(&thermal->update_work,
                         msecs_to_jiffies(THERMAL_UPDATE_INTERVAL_MS));
    
    return 0;
}

void cleanup_thermal_monitoring(struct anarchy_device *adev)
{
    struct thermal_profile *thermal = &adev->thermal;
    
    cancel_delayed_work_sync(&thermal->update_work);
}

EXPORT_SYMBOL_GPL(init_thermal_monitoring);
EXPORT_SYMBOL_GPL(cleanup_thermal_monitoring);
