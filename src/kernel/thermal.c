#include <linux/module.h>
#include <linux/thermal.h>
#include "include/common.h"

/* Lenovo P16 thermal configuration */
#define P16_FAN_MIN_SPEED    30  /* 30% minimum */
#define P16_FAN_MAX_SPEED    100 /* 100% maximum */
#define P16_TEMP_TARGET      75  /* 75°C target */
#define P16_TEMP_CRITICAL    87  /* 87°C critical */

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
    int i;
    struct power_profile *profile = &adev->current_profile;

    /* Adjust fan speed based on temperature */
    if (temp >= P16_TEMP_CRITICAL) {
        /* Emergency cooling */
        profile->fan_speed = P16_FAN_MAX_SPEED;
        profile->power_limit = 150; /* Drop to base TGP */
    } else if (temp >= P16_TEMP_TARGET) {
        /* Gradual fan speed increase */
        for (i = 0; i < ARRAY_SIZE(fan_speed_steps); i++) {
            if (temp < (P16_TEMP_TARGET + (i * 2))) {
                profile->fan_speed = fan_speed_steps[i];
                break;
            }
        }
    } else {
        /* Normal operation */
        profile->fan_speed = fan_speed_steps[2]; /* Low speed */
    }

    /* Apply changes */
    anarchy_gpu_set_fan_speed(adev, profile->fan_speed);
    anarchy_gpu_set_power_limit(adev, profile->power_limit);
}
