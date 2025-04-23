#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include "include/ring.h"
#include "include/anarchy_device.h"
#include "include/common.h"

/* DMA descriptor structure */
struct dma_desc {
    dma_addr_t addr;
    u32 size;
    u32 flags;
    u32 next;
};

/* DMA ring buffer */
struct dma_ring {
    struct dma_desc *descs;
    dma_addr_t desc_dma;
    void **buffers;
    dma_addr_t *buffer_dmas;
    unsigned int size;
    unsigned int head;
    unsigned int tail;
    spinlock_t lock;
};

/* Ring buffer registers */
#define RING_REG_BASE          0x20000
#define RING_CHANNEL_OFFSET    0x1000
#define RING_DMA_DESC_ADDR     0x100
#define RING_DMA_START         0x104
#define RING_STATUS            0x108

static int setup_dma_ring(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    struct dma_ring *dma;
    int i;

    dma = kzalloc(sizeof(*dma), GFP_KERNEL);
    if (!dma)
        return -ENOMEM;

    /* Allocate DMA descriptors */
    dma->size = 32;
    dma->descs = dma_alloc_coherent(&adev->pdev->dev,
                                   dma->size * sizeof(struct dma_desc),
                                   &dma->desc_dma, GFP_KERNEL);
    if (!dma->descs) {
        kfree(dma);
        return -ENOMEM;
    }

    /* Allocate DMA buffers */
    dma->buffers = kcalloc(dma->size, sizeof(void *), GFP_KERNEL);
    dma->buffer_dmas = kcalloc(dma->size, sizeof(dma_addr_t), GFP_KERNEL);
    if (!dma->buffers || !dma->buffer_dmas) {
        if (dma->buffers)
            kfree(dma->buffers);
        if (dma->buffer_dmas)
            kfree(dma->buffer_dmas);
        dma_free_coherent(&adev->pdev->dev,
                         dma->size * sizeof(struct dma_desc),
                         dma->descs, dma->desc_dma);
        kfree(dma);
        return -ENOMEM;
    }

    /* Initialize DMA buffers */
    for (i = 0; i < dma->size; i++) {
        dma->buffers[i] = dma_alloc_coherent(&adev->pdev->dev, PAGE_SIZE,
                                            &dma->buffer_dmas[i], GFP_KERNEL);
        if (!dma->buffers[i]) {
            while (--i >= 0) {
                dma_free_coherent(&adev->pdev->dev, PAGE_SIZE,
                                dma->buffers[i], dma->buffer_dmas[i]);
            }
            kfree(dma->buffers);
            kfree(dma->buffer_dmas);
            dma_free_coherent(&adev->pdev->dev,
                             dma->size * sizeof(struct dma_desc),
                             dma->descs, dma->desc_dma);
            kfree(dma);
            return -ENOMEM;
        }

        /* Initialize descriptor */
        dma->descs[i].addr = dma->buffer_dmas[i];
        dma->descs[i].size = PAGE_SIZE;
        dma->descs[i].flags = 0;
        dma->descs[i].next = (i + 1) % dma->size;
    }

    spin_lock_init(&dma->lock);
    ring->dma = dma;
    return 0;
}

static void cleanup_dma_ring(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    struct dma_ring *dma = ring->dma;
    int i;

    if (!dma)
        return;

    /* Free DMA buffers */
    if (dma->buffers && dma->buffer_dmas) {
        for (i = 0; i < dma->size; i++) {
            if (dma->buffers[i]) {
                dma_free_coherent(&adev->pdev->dev, PAGE_SIZE,
                                dma->buffers[i], dma->buffer_dmas[i]);
            }
        }
    }

    kfree(dma->buffers);
    kfree(dma->buffer_dmas);

    /* Free DMA descriptors */
    if (dma->descs) {
        dma_free_coherent(&adev->pdev->dev,
                         dma->size * sizeof(struct dma_desc),
                         dma->descs, dma->desc_dma);
    }

    kfree(dma);
    ring->dma = NULL;
}

static int submit_dma_transfer(struct anarchy_device *adev,
                             struct anarchy_ring *ring,
                             struct anarchy_transfer *transfer)
{
    struct dma_ring *dma = ring->dma;
    unsigned int next_head;
    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&dma->lock, flags);

    /* Check if ring is full */
    next_head = (dma->head + 1) % dma->size;
    if (next_head == dma->tail) {
        ret = -EBUSY;
        goto unlock;
    }

    /* Copy data to DMA buffer */
    memcpy(dma->buffers[dma->head], transfer->buffer,
           min_t(size_t, transfer->size, PAGE_SIZE));

    /* Update descriptor */
    dma->descs[dma->head].size = transfer->size;
    dma->descs[dma->head].flags = transfer->flags;

    /* Start DMA transfer */
    if (ring->is_tx) {
        writel(dma->desc_dma + dma->head * sizeof(struct dma_desc),
               adev->mmio_base + RING_DMA_DESC_ADDR);
        writel(1, adev->mmio_base + RING_DMA_START);
    }

    dma->head = next_head;

unlock:
    spin_unlock_irqrestore(&dma->lock, flags);
    return ret;
}

int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    int ret;

    if (!adev || !ring)
        return -EINVAL;

    /* Initialize base ring structure */
    ring->adev = adev;
    ring->state = ANARCHY_RING_STATE_STOPPED;
    ring->head = 0;
    ring->tail = 0;
    ring->is_tx = false;
    ring->dma = NULL;

    /* Initialize synchronization */
    spin_lock_init(&ring->lock);
    init_waitqueue_head(&ring->wait);

    /* Initialize statistics */
    atomic_set(&ring->bytes_transferred, 0);
    atomic_set(&ring->transfer_errors, 0);
    atomic_set(&ring->error_count, 0);
    atomic_set(&ring->pending, 0);

    /* Setup DMA ring */
    ret = setup_dma_ring(adev, ring);
    if (ret)
        return ret;

    return 0;
}

void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    if (!adev || !ring)
        return;

    /* Stop the ring if running */
    anarchy_ring_stop(adev, ring);

    /* Cleanup DMA resources */
    cleanup_dma_ring(adev, ring);

    /* Free transfer queue */
    kfree(ring->transfers);
}

int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx)
{
    if (!adev || !ring)
        return -EINVAL;

    ring->is_tx = tx;
    ring->state = ANARCHY_RING_STATE_RUNNING;
    ring->head = 0;
    ring->tail = 0;

    return 0;
}

void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    if (!adev || !ring)
        return;

    ring->state = ANARCHY_RING_STATE_STOPPED;
    ring->head = 0;
    ring->tail = 0;
}

int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         void *data, size_t size, struct anarchy_transfer *transfer)
{
    int ret;

    if (!adev || !ring || !data || !size || !transfer)
        return -EINVAL;

    if (ring->state != ANARCHY_RING_STATE_RUNNING)
        return -EIO;

    /* Initialize transfer */
    transfer->buffer = data;
    transfer->size = size;
    transfer->flags = 0;

    /* Submit DMA transfer */
    ret = submit_dma_transfer(adev, ring, transfer);
    if (ret)
        return ret;

    atomic_inc(&ring->pending);
    return 0;
}

void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer)
{
    if (!adev || !ring || !transfer)
        return;

    atomic_dec(&ring->pending);
    wake_up(&ring->wait);
}

EXPORT_SYMBOL_GPL(anarchy_ring_init);
EXPORT_SYMBOL_GPL(anarchy_ring_cleanup);
EXPORT_SYMBOL_GPL(anarchy_ring_start);
EXPORT_SYMBOL_GPL(anarchy_ring_stop);
EXPORT_SYMBOL_GPL(anarchy_ring_transfer);
EXPORT_SYMBOL_GPL(anarchy_ring_complete);
