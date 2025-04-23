#ifndef ANARCHY_GAME_OPT_H
#define ANARCHY_GAME_OPT_H

#include <linux/types.h>
#include "forward.h"
#include "anarchy_device.h"

/* Game compatibility flags */
#define GAME_COMPAT_STEAM      (1 << 0)
#define GAME_COMPAT_BLIZZARD   (1 << 1)
#define GAME_COMPAT_EA         (1 << 2)
#define GAME_COMPAT_EPIC       (1 << 3)
#define GAME_COMPAT_RIOT       (1 << 4)

/* Game profile structure */
struct game_profile {
    const char *name;
    u32 dma_batch_size;
    u32 texture_buffer_size;
    u32 command_buffer_size;
    bool low_latency_mode;
};

/* Main optimization function */
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_name);

#endif /* ANARCHY_GAME_OPT_H */
