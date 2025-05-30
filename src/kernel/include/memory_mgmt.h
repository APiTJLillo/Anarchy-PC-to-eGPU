#ifndef ANARCHY_MEMORY_MGMT_H
#define ANARCHY_MEMORY_MGMT_H

#include <linux/types.h>
#include "dma_types.h"

/* Memory regions for game compatibility */
#define GAME_TEXTURE_CACHE_SIZE    (512 * 1024 * 1024)  /* 512MB texture cache */
#define GAME_COMMAND_BUFFER_SIZE   (32 * 1024 * 1024)   /* 32MB command buffer */
#define GAME_VERTEX_BUFFER_SIZE    (256 * 1024 * 1024)  /* 256MB vertex buffer */
#define GAME_SHADER_CACHE_SIZE     (128 * 1024 * 1024)  /* 128MB shader cache */

/* Memory access patterns */
#define ACCESS_PATTERN_SEQUENTIAL  0
#define ACCESS_PATTERN_RANDOM      1
#define ACCESS_PATTERN_STREAMING   2

/* Memory allocation flags */
#define MEM_FLAG_CACHED    (1 << 0)  /* Allocate cached memory */
#define MEM_FLAG_UNCACHED  (1 << 1)  /* Allocate uncached memory */
#define MEM_FLAG_WC        (1 << 2)  /* Write combining memory */
#define MEM_FLAG_SECURE    (1 << 3)  /* Secure memory */

/* Memory priorities - map to DMA priorities */
#define PRIORITY_TEXTURE   DMA_PRIO_TEXTURE  /* Highest - for texture streaming */
#define PRIORITY_GEOMETRY  DMA_PRIO_HIGH     /* High - for geometry data */
#define PRIORITY_SHADER    DMA_PRIO_NORMAL   /* Medium - for shader programs */
#define PRIORITY_OTHER     DMA_PRIO_LOW      /* Low - for miscellaneous data */

/* Transfer modes */
#define TRANSFER_MODE_NORMAL      0  /* Standard transfer */
#define TRANSFER_MODE_LOWLATENCY  1  /* Low latency for time-critical data */
#define TRANSFER_MODE_BULK        2  /* Bulk transfer for large datasets */

/* Memory region flags */
#define REGION_FLAG_CACHED       (1 << 0)  /* Region should be cached */
#define REGION_FLAG_WRITEBACK    (1 << 1)  /* Write-back caching */
#define REGION_FLAG_COHERENT     (1 << 2)  /* Coherent memory access */
#define REGION_FLAG_STREAMING    (1 << 3)  /* Optimized for streaming */

struct game_memory_region {
    u64 base_addr;
    u64 size;
    u32 flags;
    u8  priority;
    u8  access_pattern;
    u8  transfer_mode;
    u8  reserved;
};

/* Memory management configuration */
struct memory_mgmt_config {
    struct game_memory_region texture_region;
    struct game_memory_region command_region;
    struct game_memory_region vertex_region;
    struct game_memory_region shader_region;
    bool texture_streaming_enabled;
    bool command_batching_enabled;
    u32 batch_size;
    u32 prefetch_size;
};

#endif /* ANARCHY_MEMORY_MGMT_H */
