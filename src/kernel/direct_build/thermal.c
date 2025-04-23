#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include "include/common.h"
#include "include/power_mgmt.h"
#include "include/perf_monitor.h"
#include "include/thermal.h"
#include "include/anarchy_device.h"
#include "include/thermal_forward.h"

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

static void adjust_thermal_profile(struct anarchy_device *adev, int temp)
{
    struct power_profile profile;
    int i, target_fan_speed;
    
    /* Get current power profile */
    anarchy_power_get_profile(adev, &profile);

    /* Adjust fan speed based on temperature */
    if (temp >= THERMAL_THRESHOLD_CRITICAL) {
        /* Emergency cooling */
        target_fan_speed = fan_speed_steps[7]; /* Maximum */
        profile.power_limit = 150; /* Drop to base TGP */
        profile.dynamic_control = false;
    } else if (temp >= THERMAL_THRESHOLD_WARNING) {
        /* Aggressive cooling */
        target_fan_speed = fan_speed_steps[6]; /* Very high */
        profile.power_limit = 175; /* Reduced power limit */
    } else if (temp >= THERMAL_THRESHOLD_NORMAL) {
        /* Gradual fan speed increase */
        for (i = 0; i < ARRAY_SIZE(fan_speed_steps); i++) {
            if (temp < (THERMAL_THRESHOLD_NORMAL + (i * 2000))) { /* Convert to millidegrees */
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
    struct thermal_profile *profile = container_of(to_delayed_work(work),
                                                 struct thermal_profile,
                                                 update_work);
    struct anarchy_device *adev = container_of(profile, struct anarchy_device,
                                             thermal_profile);
    struct perf_state perf_state;
    unsigned long flags;
    u32 temp;

    /* Get current performance state which includes temperature */
    anarchy_perf_get_state(adev, &perf_state);
    temp = perf_state.temperature;

    spin_lock_irqsave(&profile->lock, flags);
    
    profile->current_temp = temp;
    if (temp > profile->max_temp) {
        profile->max_temp = temp;
    }

    /* Adjust thermal profile based on temperature */
    adjust_thermal_profile(adev, temp);

    /* Check thresholds and trigger callbacks if needed */
    if (temp >= profile->critical_threshold) {
        if (profile->critical_callback)
            profile->critical_callback(adev);
    } else if (temp >= profile->warning_threshold) {
        if (profile->warning_callback)
            profile->warning_callback(adev);
    }

    spin_unlock_irqrestore(&profile->lock, flags);

    /* Reschedule if monitoring is still enabled */
    if (profile->monitoring_enabled) {
        queue_delayed_work(profile->wq, &profile->update_work,
                          msecs_to_jiffies(THERMAL_MONITOR_INTERVAL));
    }
}

int init_thermal_monitoring(struct anarchy_device *adev)
{
    struct thermal_profile *profile = &adev->thermal_profile;

    /* Initialize thermal profile */
    profile->monitoring_enabled = false;
    profile->current_temp = 0;
    profile->max_temp = 0;
    profile->warning_threshold = THERMAL_THRESHOLD_WARNING;
    profile->critical_threshold = THERMAL_THRESHOLD_CRITICAL;
    profile->target_fan_speed = fan_speed_steps[2]; /* Start at low speed */
    spin_lock_init(&profile->lock);

    /* Create workqueue */
    profile->wq = create_singlethread_workqueue("anarchy_thermal");
    if (!profile->wq) {
        dev_err(&adev->pdev->dev, "Failed to create thermal workqueue\n");
        return -ENOMEM;
    }

    /* Initialize work */
    INIT_DELAYED_WORK(&profile->update_work, thermal_update_work);

    /* Start monitoring */
    profile->monitoring_enabled = true;
    queue_delayed_work(profile->wq, &profile->update_work,
                      msecs_to_jiffies(THERMAL_MONITOR_INTERVAL));

    return 0;
}

void cleanup_thermal_monitoring(struct anarchy_device *adev)
{
    struct thermal_profile *profile = &adev->thermal_profile;

    /* Stop monitoring */
    profile->monitoring_enabled = false;
    cancel_delayed_work_sync(&profile->update_work);

    /* Destroy workqueue */
    if (profile->wq) {
        destroy_workqueue(profile->wq);
        profile->wq = NULL;
    }
}

EXPORT_SYMBOL_GPL(init_thermal_monitoring);
EXPORT_SYMBOL_GPL(cleanup_thermal_monitoring);
