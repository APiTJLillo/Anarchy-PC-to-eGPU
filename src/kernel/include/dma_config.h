#ifndef ANARCHY_DMA_CONFIG_H
#define ANARCHY_DMA_CONFIG_H

#include <linux/types.h>
#include "forward.h"

/* DMA transfer modes */
enum anarchy_dma_mode {
    ANARCHY_DMA_MODE_NORMAL = 0,
    ANARCHY_DMA_MODE_STREAMING,
    ANARCHY_DMA_MODE_LOW_LATENCY,
    ANARCHY_DMA_MODE_HIGH_BANDWIDTH
};

/* DMA channel configuration */
struct anarchy_dma_channel {
    unsigned int id;
    unsigned int priority;
    enum anarchy_dma_mode mode;
    unsigned int max_payload;
    unsigned int ring_size;
    bool is_active;
};

/* DMA configuration for gaming workloads */
struct anarchy_dma_config {
    unsigned int num_channels;
    unsigned int batch_size;
    unsigned int buffer_size;
    bool low_latency_mode;
    bool streaming_enabled;
    struct anarchy_dma_channel channels[12];  /* Max 12 channels */
};

/* DMA management functions */
int anarchy_dma_init_gaming(struct anarchy_device *adev);
int anarchy_dma_configure_for_game(struct anarchy_device *adev, const char *game);
void anarchy_dma_optimize_transfers(struct anarchy_device *adev);
void anarchy_dma_cleanup(struct anarchy_device *adev);

/* Channel management */
int anarchy_dma_allocate_channel(struct anarchy_device *adev, enum anarchy_dma_mode mode);
void anarchy_dma_free_channel(struct anarchy_device *adev, int channel_id);
int anarchy_dma_set_channel_priority(struct anarchy_device *adev, int channel_id, 
                                   unsigned int priority);

#endif /* ANARCHY_DMA_CONFIG_H */
