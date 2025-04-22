#ifndef ANARCHY_COMMAND_PROC_H
#define ANARCHY_COMMAND_PROC_H

#include <linux/types.h>
#include "forward.h"

/* Command batch structure */
struct command_batch {
    void *cmds;
    size_t size;
    unsigned int num_cmds;
    bool needs_sync;
    bool is_compute;
};

/* Command processor state */
struct command_processor {
    struct anarchy_device *adev;
    atomic_t active_batches;
    spinlock_t lock;
    wait_queue_head_t wait;
    void *cmd_ring_base;
    unsigned int ring_size;
    unsigned int write_ptr;
    unsigned int read_ptr;
};

/* Command processor functions */
int init_command_processor(struct anarchy_device *adev);
void cleanup_command_processor(struct anarchy_device *adev);
int process_game_command(struct anarchy_device *adev, struct command_batch *batch);
void optimize_command_processing(struct anarchy_device *adev, const char *game_name);

#endif /* ANARCHY_COMMAND_PROC_H */
