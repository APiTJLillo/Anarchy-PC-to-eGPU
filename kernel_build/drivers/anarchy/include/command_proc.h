#ifndef ANARCHY_COMMAND_PROC_H
#define ANARCHY_COMMAND_PROC_H

#include <linux/types.h>
#include "forward.h"
#include "command_types.h"

/* Command processor initialization and cleanup */
int init_command_processor(struct anarchy_device *adev);
void cleanup_command_processor(struct anarchy_device *adev);

/* Command processing */
int process_game_command(struct anarchy_device *adev, struct command_batch *batch);
int process_command_batch_immediate(struct anarchy_device *adev, struct command_batch *batch);
void merge_command_batch(struct command_batch *dst, struct command_batch *src);

/* Command processor optimization */
void optimize_command_processing(struct anarchy_device *adev, u32 load);

#endif /* ANARCHY_COMMAND_PROC_H */
