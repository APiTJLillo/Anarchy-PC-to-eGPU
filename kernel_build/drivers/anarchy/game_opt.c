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
#include <linux/module.h>
#include "include/dma_types.h"

/* Default power limit for games */
#define DEFAULT_POWER_LIMIT 175

/* Game optimization profiles */
static const struct game_profile game_profiles[] = {
    {
        .name = "default",
        .dma_batch_size = 256,
        .texture_buffer_size = 64 * 1024 * 1024,  /* 64MB */
        .command_buffer_size = 1 * 1024 * 1024,   /* 1MB */
        .low_latency_mode = true,
    },
    {
        .name = "high_performance",
        .dma_batch_size = 512,
        .texture_buffer_size = 128 * 1024 * 1024, /* 128MB */
        .command_buffer_size = 2 * 1024 * 1024,   /* 2MB */
        .low_latency_mode = true,
    },
    {
        .name = "memory_optimized",
        .dma_batch_size = 128,
        .texture_buffer_size = 32 * 1024 * 1024,  /* 32MB */
        .command_buffer_size = 512 * 1024,        /* 512KB */
        .low_latency_mode = false,
    },
};

/* Set up memory regions based on profile */
static int setup_memory_regions(struct anarchy_device *adev,
                              const struct game_profile *profile)
{
    struct game_memory_region *texture_region, *command_region;
    u32 flags = REGION_FLAG_COHERENT;

    if (profile->low_latency_mode)
        flags |= REGION_FLAG_LOWLATENCY;

    /* Set up texture memory region */
    texture_region = setup_game_memory_region(adev, profile->texture_buffer_size, flags);
    if (IS_ERR(texture_region))
        return PTR_ERR(texture_region);

    /* Set up command memory region */
    command_region = setup_game_memory_region(adev, profile->command_buffer_size,
                                            REGION_FLAG_COHERENT | REGION_FLAG_CACHED);
    if (IS_ERR(command_region)) {
        cleanup_game_memory_region(adev, texture_region);
        return PTR_ERR(command_region);
    }

    /* Store regions in compatibility layer */
    if (adev->compat_layer) {
        cleanup_game_memory_region(adev, adev->compat_layer->texture_region);
        cleanup_game_memory_region(adev, adev->compat_layer->command_region);
        adev->compat_layer->texture_region = texture_region;
        adev->compat_layer->command_region = command_region;
    }

    return 0;
}

/* Find game profile by name */
static const struct game_profile *find_game_profile(const char *name)
{
    int i;

    if (!name)
        return &game_profiles[0];  /* Return default profile */

    for (i = 0; i < ARRAY_SIZE(game_profiles); i++) {
        if (strcmp(game_profiles[i].name, name) == 0)
            return &game_profiles[i];
    }

    return &game_profiles[0];  /* Return default if not found */
}

/* Apply game optimization profile */
static int apply_game_profile(struct anarchy_device *adev,
                            const struct game_profile *profile)
{
    int ret;

    if (!adev || !profile)
        return -EINVAL;

    /* Configure DMA settings */
    ret = anarchy_dma_set_device_priority(adev, 0, ANARCHY_DMA_PRIO_HIGH);
    if (ret)
        return ret;

    /* Set up memory regions */
    ret = setup_memory_regions(adev, profile);
    if (ret)
        return ret;

    /* Configure device parameters */
    adev->dma_batch_size = profile->dma_batch_size;
    adev->low_latency_mode = profile->low_latency_mode;

    dev_info(&adev->pdev->dev, "Applied game profile: %s\n", profile->name);
    return 0;
}

/* Main optimization function */
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_name)
{
    const struct game_profile *profile;
    int ret;

    if (!adev)
        return -EINVAL;

    profile = find_game_profile(game_name);
    ret = apply_game_profile(adev, profile);
    if (ret)
        return ret;

    dev_info(&adev->pdev->dev, "Game optimization complete for %s\n", game_name);
    return 0;
}

EXPORT_SYMBOL_GPL(anarchy_optimize_for_game);
