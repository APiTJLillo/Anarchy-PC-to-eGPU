#ifndef ANARCHY_GAME_COMPAT_H
#define ANARCHY_GAME_COMPAT_H

#include <linux/types.h>
#include "forward.h"
#include "game_compat_types.h"

/* Game compatibility initialization and cleanup */
int init_game_compatibility(struct anarchy_device *adev);
void cleanup_game_compatibility(struct anarchy_device *adev);
int init_game_specific(struct anarchy_device *adev, const char *game_name);

/* Game optimization interface */
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_name);

/* Memory region management */
struct game_memory_region *setup_game_memory_region(struct anarchy_device *adev,
                                                  size_t size, u32 flags);
void cleanup_game_memory_region(struct anarchy_device *adev,
                              struct game_memory_region *region);

#endif /* ANARCHY_GAME_COMPAT_H */
