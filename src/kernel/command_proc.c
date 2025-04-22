#include <linux/module.h>
#include <linux/slab.h>
#include "include/anarchy_device.h"
#include "include/command_types.h"
#include "include/command_proc.h"
#include "include/dma_config.h"
#include "include/dma.h"

/* Initialize command processor */
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

    cp->cmd_wq = alloc_workqueue("anarchy_cmd_wq", WQ_HIGHPRI | WQ_UNBOUND, 1);
    if (!cp->cmd_wq) {
        kfree(cp);
        return -ENOMEM;
    }

    adev->cmd_proc = cp;
    return 0;
}

/* Command batch merging */
void merge_command_batch(struct command_batch *dst, struct command_batch *src)
{
    struct command_batch *last;

    if (!dst->data) {
        /* First command, just copy */
        dst->category = src->category;
        dst->flags = src->flags;
        dst->data = src->data;
        dst->total_size = src->total_size;
        return;
    }

    /* Find end of chain */
    last = dst;
    while (last->next)
        last = last->next;

    /* Link new batch */
    last->next = kmemdup(src, sizeof(*src), GFP_ATOMIC);
    if (last->next)
        dst->total_size += src->total_size;
}

/* Process game commands */
int process_game_command(struct anarchy_device *adev, struct command_batch *batch)
{
    struct command_processor *cp = adev->cmd_proc;
    unsigned long flags;
    int ret = 0;

    if (!cp || !batch)
        return -EINVAL;

    /* Handle low latency texture commands immediately */
    if (batch->category == CMD_CAT_TEXTURE &&
        (batch->flags & CMD_FLAG_LOWLAT)) {
        
        ret = anarchy_dma_transfer_priority(adev, batch->data,
                                          batch->total_size,
                                          PRIORITY_TEXTURE);
        if (ret)
            return ret;
    }

    spin_lock_irqsave(&cp->lock, flags);
    
    /* Try to batch commands when possible */
    if (cp->batching_enabled && !(batch->flags & CMD_FLAG_NOSYNC)) {
        
        if (!cp->current_batch) {
            cp->current_batch = kzalloc(sizeof(*cp->current_batch), GFP_ATOMIC);
            if (!cp->current_batch) {
                spin_unlock_irqrestore(&cp->lock, flags);
                return -ENOMEM;
            }
        }

        merge_command_batch(cp->current_batch, batch);
    } else {
        /* Process immediately if batching disabled or NOSYNC flag set */
        ret = process_command_batch_immediate(adev, batch);
    }

    spin_unlock_irqrestore(&cp->lock, flags);
    return ret;
}

/* Immediate command batch processing */
int process_command_batch_immediate(struct anarchy_device *adev, struct command_batch *batch)
{
    int ret = 0;

    /* TODO: Implement actual command processing
     * For now just do a DMA transfer */
    ret = anarchy_dma_transfer(adev, batch->data, batch->total_size);

    return ret;
}

/* Optimize command processing based on load */
void optimize_command_processing(struct anarchy_device *adev, u32 load)
{
    struct command_processor *cp = adev->cmd_proc;
    unsigned long flags;

    if (!cp)
        return;

    spin_lock_irqsave(&cp->lock, flags);

    if (load > 80) {
        /* High load optimizations */
        cp->batch_size = 512;       /* Larger batches for better throughput */
        cp->low_latency_mode = true;/* Enable low latency mode */
    } else {
        /* Normal load settings */
        cp->batch_size = 256;
        cp->low_latency_mode = true;
    }

    spin_unlock_irqrestore(&cp->lock, flags);
}

/* Cleanup */
void cleanup_command_processor(struct anarchy_device *adev)
{
    struct command_processor *cp = adev->cmd_proc;
    struct command_batch *batch, *next;

    if (!cp)
        return;

    if (cp->cmd_wq)
        destroy_workqueue(cp->cmd_wq);

    /* Free any pending command batches */
    batch = cp->current_batch;
    while (batch) {
        next = batch->next;
        kfree(batch);
        batch = next;
    }

    kfree(cp);
    adev->cmd_proc = NULL;
}

EXPORT_SYMBOL_GPL(init_command_processor);
EXPORT_SYMBOL_GPL(cleanup_command_processor);
EXPORT_SYMBOL_GPL(process_game_command);
EXPORT_SYMBOL_GPL(optimize_command_processing);
