#ifndef ANARCHY_GAME_OPT_H
#define ANARCHY_GAME_OPT_H

#include <linux/types.h>
#include "forward.h"

/* Game compatibility flags */
#define GAME_COMPAT_STEAM      (1 << 0)
#define GAME_COMPAT_BLIZZARD   (1 << 1)
#define GAME_COMPAT_EA         (1 << 2)
#define GAME_COMPAT_EPIC       (1 << 3)
#define GAME_COMPAT_RIOT       (1 << 4)

/* Game profiles */
struct game_profile {
    const char *name;
    u32 optimization_flags;
    u32 power_limit;
    u32 dma_channels;
    bool low_latency;
};

/* Game profile functions */
const struct game_profile *find_game_profile(const char *game_name);
int apply_game_profile(struct anarchy_device *adev, const struct game_profile *profile);

#endif /* ANARCHY_GAME_OPT_H */
