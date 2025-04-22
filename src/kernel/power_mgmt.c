#include <linux/module.h>
#include <linux/kernel.h>
#include "include/gpu_ids.h"
#include "include/gpu_config.h"
#include "include/gpu_emu.h"

/* Dynamic power states */
enum power_state {
    POWER_STATE_LOW = 0,     /* 150W - Light gaming/desktop */
    POWER_STATE_NORMAL = 1,  /* 175W - Most gaming workloads */
    POWER_STATE_BOOST = 2,   /* 200W - Intense gaming/benchmarks */
    POWER_STATE_MAX = 3,     /* 250W - Maximum allowed */
};

/* Mobile RTX 4090 specific power management */
static const struct power_profile mobile_power_profiles[] = {
    [POWER_PROFILE_BATTERY] = {
        .power_limit = 150,     /* Conservative 150W limit */
        .clock_limit = 1395,    /* Base clock only */
        .fan_profile = FAN_PROFILE_QUIET,
        .memory_clock = 19000,  /* Reduced memory clock */
    },
    [POWER_PROFILE_BALANCED] = {
        .power_limit = 175,     /* Standard 175W TGP */
        .clock_limit = 2040,    /* Allow boost clock */
        .fan_profile = FAN_PROFILE_BALANCED,
        .memory_clock = 21000,  /* Full memory speed */
    },
    [POWER_PROFILE_PERFORMANCE] = {
        .power_limit = 200,     /* Enhanced 200W limit */
        .clock_limit = 2040,    /* Maximum boost */
        .fan_profile = FAN_PROFILE_PERFORMANCE,
        .memory_clock = 21000,  /* Full memory speed */
    }
};

/* Power management for mobile RTX 4090 */
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, u32 power_limit)
{
    if (power_limit < MIN_POWER_LIMIT || power_limit > MAX_POWER_LIMIT)
        return -EINVAL;

    adev->power_limit = power_limit;
    
    /* Update power limit in hardware registers */
    if (adev->mmio_base) {
        writel(power_limit, adev->mmio_base + PWR_LIMIT_OFFSET);
        writel(1, adev->mmio_base + PWR_LIMIT_UPDATE);
    }

    return 0;
}

/* Game-optimized power profile selection */
int anarchy_gpu_set_game_profile(struct anarchy_device *adev, const char *game_name)
{
    int i;
    const struct game_optimization *opt = NULL;

    /* Find game-specific optimization if available */
    for (i = 0; i < ARRAY_SIZE(game_defaults); i++) {
        if (strcmp(game_defaults[i].game_name, game_name) == 0) {
            opt = &game_defaults[i];
            break;
        }
    }

    if (opt) {
        /* Apply game-specific power settings */
        anarchy_gpu_set_power_limit(adev, opt->min_power_limit);
        adev->texture_cache_size = opt->texture_cache_size;
        adev->dynamic_power = opt->dynamic_power;
        adev->texture_streaming = opt->texture_streaming;
    } else {
        /* Apply default gaming profile */
        anarchy_gpu_set_power_limit(adev, DEFAULT_POWER_LIMIT);
        adev->texture_cache_size = GAME_TEXTURE_CACHE_SIZE;
        adev->dynamic_power = true;
        adev->texture_streaming = true;
    }

    return 0;
}
