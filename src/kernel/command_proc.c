#include <linux/module.h>
#include <linux/slab.h>
#include "include/command_proc.h"
#include "include/gpu_emu.h"

int init_command_processor(struct anarchy_device *adev)
{
    struct command_processor *cp;

    cp = kzalloc(sizeof(*cp), GFP_KERNEL);
    if (!cp)
        return -ENOMEM;

    cp->batch_size = 256;  /* Start with 256 commands per batch */
    cp->batching_enabled = true;
    cp->low_latency_mode = true;
    spin_lock_init(&cp->lock);

    adev->command_processor = cp;
    return 0;
}

int process_game_command(struct anarchy_device *adev, struct command_batch *batch)
{
    struct command_processor *cp = adev->command_processor;
    unsigned long flags;
    int ret = 0;

    if (!cp || !batch)
        return -EINVAL;

    spin_lock_irqsave(&cp->lock, flags);

    /* Fast path for texture commands in games like WRC2 */
    if (batch->category == CMD_CAT_TEXTURE && 
        (batch->flags & CMD_FLAG_LOWLAT)) {
        /* Use high-priority DMA channel */
        ret = anarchy_dma_transfer_priority(adev, batch->data, 
                                          batch->total_size, 
                                          PRIORITY_TEXTURE);
        goto out_unlock;
    }

    /* Normal command processing */
    if (cp->batching_enabled && !(batch->flags & CMD_FLAG_NOSYNC)) {
        /* Add to current batch */
        if (!cp->current_batch) {
            cp->current_batch = kzalloc(sizeof(*cp->current_batch), GFP_ATOMIC);
            if (!cp->current_batch) {
                ret = -ENOMEM;
                goto out_unlock;
            }
        }
        /* Merge commands if possible */
        merge_command_batch(cp->current_batch, batch);
    } else {
        /* Process immediately */
        ret = process_command_batch_immediate(adev, batch);
    }

out_unlock:
    spin_unlock_irqrestore(&cp->lock, flags);
    return ret;
}

void optimize_command_processing(struct anarchy_device *adev, const char *game_name)
{
    struct command_processor *cp = adev->command_processor;
    
    if (!cp)
        return;

    if (strstr(game_name, "WRC2")) {
        /* WRC2 optimizations */
        cp->batch_size = 512;       /* Larger batches for better throughput */
        cp->low_latency_mode = true;/* Enable low latency mode */
    } else {
        /* Default gaming optimizations */
        cp->batch_size = 256;
        cp->low_latency_mode = true;
    }
}
