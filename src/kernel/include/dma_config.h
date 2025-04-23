#ifndef ANARCHY_DMA_CONFIG_H
#define ANARCHY_DMA_CONFIG_H

#include "dma_types.h"

/* DMA configuration functions */
int anarchy_dma_init(void);
void anarchy_dma_shutdown(void);

/* DMA channel management */
int anarchy_dma_allocate_channel(enum anarchy_dma_priority priority);
void anarchy_dma_free_channel(int channel);
int anarchy_dma_set_priority(int channel, enum anarchy_dma_priority priority);

/* DMA transfer management */
int anarchy_dma_start_transfer(int channel, void *src, void *dst, size_t size, 
                             enum anarchy_dma_direction direction);
int anarchy_dma_pause_transfer(int channel);
int anarchy_dma_resume_transfer(int channel);
int anarchy_dma_stop_transfer(int channel);
enum anarchy_dma_status anarchy_dma_get_status(int channel);

#endif /* ANARCHY_DMA_CONFIG_H */
