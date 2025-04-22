#ifndef ANARCHY_COMMAND_TYPES_H
#define ANARCHY_COMMAND_TYPES_H

#include <linux/types.h>

/* Command categories */
enum command_category {
    CMD_CAT_GENERAL = 0,
    CMD_CAT_TEXTURE,
    CMD_CAT_SHADER,
    CMD_CAT_BUFFER,
    CMD_CAT_SYNC
};

/* Command flags */
#define CMD_FLAG_LOWLAT  (1 << 0)  /* Low latency command */
#define CMD_FLAG_NOSYNC  (1 << 1)  /* Don't sync with other commands */
#define CMD_FLAG_BATCH   (1 << 2)  /* Can be batched with other commands */

/* DMA priorities */
enum dma_priority {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL,
    PRIORITY_TEXTURE,
    PRIORITY_HIGH
};

/* Command batch structure */
struct command_batch {
    enum command_category category;
    u32 flags;
    void *data;
    size_t total_size;
    struct command_batch *next;
};

/* Command processor structure */
struct command_processor {
    unsigned int batch_size;
    bool batching_enabled;
    bool low_latency_mode;
    struct command_batch *current_batch;
    struct workqueue_struct *cmd_wq;
    spinlock_t lock;
};

#endif /* ANARCHY_COMMAND_TYPES_H */
