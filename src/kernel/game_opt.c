#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include "include/anarchy_device.h"
#include "include/game_opt.h"
#include "include/command_proc.h"
#include "include/gpu_power.h"
#include "include/dma.h"
#include "include/game_compat_types.h"
#include "include/game_compat.h"

/* Default power limit for games */
#define DEFAULT_POWER_LIMIT 175

/* Game optimization profiles */
static const struct game_profile game_profiles[] = {
    {
        .name = "steam",
        .optimization_flags = GAME_COMPAT_STEAM,
        .power_limit = 200,
        .dma_channels = 8,
        .low_latency = true
    },
    {
        .name = "blizzard",
        .optimization_flags = GAME_COMPAT_BLIZZARD,
        .power_limit = 175,
        .dma_channels = 6,
        .low_latency = false
    },
    {
        .name = "ea",
        .optimization_flags = GAME_COMPAT_EA,
        .power_limit = 150,
        .dma_channels = 4,
        .low_latency = true
    }
};

const struct game_profile *find_game_profile(const char *game_name)
{
    int i;

    if (!game_name)
        return NULL;

    for (i = 0; i < ARRAY_SIZE(game_profiles); i++) {
        if (strcasecmp(game_profiles[i].name, game_name) == 0)
            return &game_profiles[i];
    }

    /* Return default profile if no match found */
    return &game_profiles[0];
}

int apply_game_profile(struct anarchy_device *adev, const struct game_profile *profile)
{
    int ret;

    if (!adev || !profile)
        return -EINVAL;

    /* Apply power limit */
    ret = anarchy_gpu_set_power_limit(adev, profile->power_limit);
    if (ret)
        return ret;

    /* Configure DMA channels */
    ret = anarchy_dma_set_channel_priority(adev, 0, PRIORITY_HIGH);
    if (ret)
        return ret;

    /* Apply memory optimizations if needed */
    if (profile->low_latency && adev->compat_layer) {
        struct game_memory_region *region = adev->compat_layer->texture_region;
        if (region)
            region->flags |= REGION_FLAG_LOWLATENCY;
    }

    dev_info(&adev->dev, "Applied game profile: %s\n", profile->name);
    return 0;
}

int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_name)
{
    const struct game_profile *profile;
    int ret;

    profile = find_game_profile(game_name);
    if (!profile)
        return -EINVAL;

    ret = apply_game_profile(adev, profile);
    if (ret)
        return ret;

    /* Set up load based optimization */
    optimize_command_processing(adev, profile->low_latency ? 80 : 50);

    dev_info(&adev->dev, "Game optimization complete for %s\n", game_name);
    return 0;
}

EXPORT_SYMBOL_GPL(anarchy_optimize_for_game);
EXPORT_SYMBOL_GPL(find_game_profile);
EXPORT_SYMBOL_GPL(apply_game_profile);
