#ifndef ANARCHY_GAME_COMPAT_H
#define ANARCHY_GAME_COMPAT_H

#include "anarchy_device.h"
#include "game_compat_types.h"

/* Game memory region functions */
struct game_memory_region *setup_game_memory_region(struct anarchy_device *adev,
                                                  size_t size, u32 flags);
void cleanup_game_memory_region(struct anarchy_device *adev,
                              struct game_memory_region *region);

/* Game compatibility functions */
int init_game_compatibility(struct anarchy_device *adev);
void cleanup_game_compatibility(struct anarchy_device *adev);
int init_game_specific(struct anarchy_device *adev, const char *game_name);

#endif /* ANARCHY_GAME_COMPAT_H */
