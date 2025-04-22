#ifndef ANARCHY_GAME_COMPAT_TYPES_H
#define ANARCHY_GAME_COMPAT_TYPES_H

#include <linux/types.h>

/* Memory region flags */
#define REGION_FLAG_LOWLATENCY  (1 << 0)
#define REGION_FLAG_WRITEBACK   (1 << 1)
#define REGION_FLAG_CACHED      (1 << 2)
#define REGION_FLAG_COHERENT    (1 << 3)

/* Memory region sizes */
#define GAME_COMMAND_BUFFER_SIZE   (1024 * 1024)     /* 1MB */
#define GAME_TEXTURE_BUFFER_SIZE   (256 * 1024 * 1024) /* 256MB */

/* Memory region structure */
struct game_memory_region {
    void *vaddr;
    dma_addr_t dma_addr;
    size_t size;
    u32 flags;
};

/* Game compatibility layer */
struct game_compat_layer {
    struct game_memory_region *texture_region;
    struct game_memory_region *command_region;
    struct game_profile *profile;
    void *private_data;
};

#endif /* ANARCHY_GAME_COMPAT_TYPES_H */
