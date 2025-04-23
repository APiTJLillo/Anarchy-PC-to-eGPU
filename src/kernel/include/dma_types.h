#ifndef ANARCHY_DMA_TYPES_H
#define ANARCHY_DMA_TYPES_H

/* DMA priority levels */
enum anarchy_dma_priority {
    ANARCHY_DMA_PRIO_LOW = 0,
    ANARCHY_DMA_PRIO_NORMAL,
    ANARCHY_DMA_PRIO_HIGH,
    ANARCHY_DMA_PRIO_REALTIME,
    ANARCHY_DMA_PRIO_TEXTURE  /* For texture streaming */
};

/* DMA transfer status */
enum anarchy_dma_status {
    ANARCHY_DMA_STATUS_IDLE = 0,
    ANARCHY_DMA_STATUS_ACTIVE,
    ANARCHY_DMA_STATUS_ERROR,
    ANARCHY_DMA_STATUS_PAUSED
};

/* DMA transfer direction */
enum anarchy_dma_direction {
    ANARCHY_DMA_TO_DEVICE = 0,
    ANARCHY_DMA_FROM_DEVICE,
    ANARCHY_DMA_BIDIRECTIONAL
};

#endif /* ANARCHY_DMA_TYPES_H */ 