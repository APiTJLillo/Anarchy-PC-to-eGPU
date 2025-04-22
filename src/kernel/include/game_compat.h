#ifndef ANARCHY_GAME_COMPAT_H
#define ANARCHY_GAME_COMPAT_H

#include <linux/types.h>
#include "forward.h"

/* Game optimization profiles */
struct game_profile {
    const char *name;
    unsigned int core_clock;
    unsigned int mem_clock;
    unsigned int power_limit;
    unsigned int fan_speed;
    bool ray_tracing;
    bool dlss;
    unsigned int texture_compression;
};

/* Game compatibility layer */
struct game_compat_layer {
    struct anarchy_device *adev;
    struct game_profile *current_profile;
    bool anti_cheat_enabled;
    bool is_dx12;
    bool is_vulkan;
    spinlock_t lock;
};

/* Core functions */
int init_game_compatibility(struct anarchy_device *adev);
void cleanup_game_compatibility(struct anarchy_device *adev);
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game);
int anarchy_load_game_profile(struct anarchy_device *adev, const char *profile_name);

#endif /* ANARCHY_GAME_COMPAT_H */
