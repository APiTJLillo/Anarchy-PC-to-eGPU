#include <linux/module.h>
#include "include/gpu_ids.h"
#include "include/gpu_config.h"
#include "include/memory_mgmt.h"
#include "include/command_proc.h"

/* Game detection and optimization */
struct game_profile {
    const char *name;
    const char *exe_name;
    u32 optimization_flags;
    struct {
        u32 power_limit;
        u32 texture_cache;
        u32 command_batch;
        bool low_latency;
    } settings;
};

/* Known game profiles */
static const struct game_profile known_games[] = {
    {
        .name = "WRC2",
        .exe_name = "wrc2.exe",
        .optimization_flags = GAME_COMPAT_STEAM,
        .settings = {
            .power_limit = 175,      /* Default TGP */
            .texture_cache = 512,    /* 512MB texture cache */
            .command_batch = 512,    /* Large command batches */
            .low_latency = true,
        }
    },
    {
        .name = "Overwatch",
        .exe_name = "overwatch.exe",
        .optimization_flags = GAME_COMPAT_BLIZZARD,
        .settings = {
            .power_limit = 165,      /* Slightly lower TGP */
            .texture_cache = 256,    /* 256MB texture cache */
            .command_batch = 256,    /* Standard batching */
            .low_latency = true,
        }
    },
    {
        .name = "EA Sports FC 24",
        .exe_name = "fc24.exe",
        .optimization_flags = GAME_COMPAT_EA,
        .settings = {
            .power_limit = 175,      /* Default TGP */
            .texture_cache = 384,    /* 384MB texture cache */
            .command_batch = 384,    /* Larger batches */
            .low_latency = true,
        }
    }
};

/* Apply game-specific optimizations */
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_name)
{
    const struct game_profile *profile = NULL;
    int i;

    /* Find matching game profile */
    for (i = 0; i < ARRAY_SIZE(known_games); i++) {
        if (strstr(game_name, known_games[i].exe_name)) {
            profile = &known_games[i];
            break;
        }
    }

    if (!profile) {
        /* Unknown game - use balanced defaults */
        anarchy_gpu_set_power_limit(adev, DEFAULT_POWER_LIMIT);
        return 0;
    }

    /* Apply profile settings */
    anarchy_gpu_set_power_limit(adev, profile->settings.power_limit);
    optimize_command_processing(adev, profile->name);
    anarchy_dma_configure_for_game(adev, profile->name);
    
    /* Configure memory regions */
    struct game_memory_region *texture_region = adev->compat_layer->texture_region;
    if (texture_region) {
        texture_region->size = profile->settings.texture_cache * 1024 * 1024;
        texture_region->flags |= REGION_FLAG_STREAMING;
    }

    dev_info(adev->dev, "Applied optimizations for %s\n", profile->name);
    return 0;
}
