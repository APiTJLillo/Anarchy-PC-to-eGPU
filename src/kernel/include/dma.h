#ifndef ANARCHY_DMA_H
#define ANARCHY_DMA_H

#include <linux/types.h>
#include "forward.h"
#include "command_types.h"

/* DMA transfer functions */
int anarchy_dma_transfer(struct anarchy_device *adev, void *data, size_t size);
int anarchy_dma_transfer_priority(struct anarchy_device *adev, void *data,
                                size_t size, enum dma_priority priority);
int anarchy_dma_set_channel_priority(struct anarchy_device *adev, int channel,
                                   enum dma_priority priority);

#endif /* ANARCHY_DMA_H */
