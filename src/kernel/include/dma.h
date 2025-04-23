#ifndef _ANARCHY_DMA_H_
#define _ANARCHY_DMA_H_

#include <linux/types.h>
#include <linux/dma-mapping.h>
#include "anarchy_device.h"
#include "dma_types.h"

/* Basic DMA transfer functions */
dma_addr_t anarchy_dma_transfer(struct anarchy_device *adev, void *data, size_t size);
int anarchy_dma_transfer_priority(struct anarchy_device *adev, void *data,
                                size_t size, enum anarchy_dma_priority priority);
void anarchy_dma_cleanup(struct anarchy_device *adev, dma_addr_t dma_addr, size_t size);

/* Device-specific DMA functions */
int anarchy_dma_device_start_transfer(struct anarchy_device *adev, int channel,
                                    dma_addr_t addr, u32 offset, size_t size);
int anarchy_dma_set_device_priority(struct anarchy_device *adev, int channel,
                                  enum anarchy_dma_priority priority);

/* DMA optimization functions */
void anarchy_dma_optimize_transfers(struct anarchy_device *adev);

/* DMA device configuration functions */
void anarchy_dma_device_set_burst_size(struct anarchy_device *adev, int size);
void anarchy_dma_device_set_queue_depth(struct anarchy_device *adev, int depth);
void anarchy_dma_device_enable_prefetch(struct anarchy_device *adev);
void anarchy_dma_device_set_write_combining(struct anarchy_device *adev, bool enable);

#endif /* _ANARCHY_DMA_H_ */
