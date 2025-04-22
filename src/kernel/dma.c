#include <linux/module.h>
#include <linux/dma-mapping.h>
#include "include/dma_config.h"
#include "include/gpu_emu.h"

int anarchy_dma_init_gaming(struct anarchy_device *adev)
{
    /* Initialize with gaming-optimized settings */
    adev->dma_config = gaming_dma_config;
    
    /* Set up DMA ring buffers with optimal sizes */
    if (anarchy_ring_init(adev, &adev->tx_ring) ||
        anarchy_ring_init(adev, &adev->rx_ring)) {
        return -ENOMEM;
    }

    /* Configure DMA mappings for VRAM access */
    if (!dma_set_mask_and_coherent(&adev->dev, DMA_BIT_MASK(64))) {
        dev_err(&adev->dev, "64-bit DMA not available\n");
        return -EINVAL;
    }

    /* Set up streaming DMA for textures */
    adev->tx_ring.streaming = true;
    adev->tx_ring.chunk_size = TRANSFER_CHUNK_SIZE;
    adev->tx_ring.max_inflight = MAX_INFLIGHT_TRANSFERS;

    return 0;
}

int anarchy_dma_configure_for_game(struct anarchy_device *adev, const char *game_name)
{
    /* Configure optimal DMA settings based on game profile */
    if (strstr(game_name, "WRC2")) {
        /* WRC2 specific optimizations */
        adev->dma_config.prefetch_size = 16384; /* 16MB prefetch for better texture streaming */
        adev->dma_config.batch_size = 512;      /* Larger batches for texture loads */
    } else {
        /* Default gaming configuration */
        adev->dma_config = gaming_dma_config;
    }

    return anarchy_dma_apply_config(adev);
}

void anarchy_dma_optimize_transfers(struct anarchy_device *adev)
{
    /* Enable adaptive transfer size based on workload */
    adev->tx_ring.adaptive_size = true;
    adev->rx_ring.adaptive_size = true;

    /* Set up priority-based transfers */
    adev->tx_ring.priority_levels = 4;
    adev->tx_ring.texture_priority = PRIORITY_TEXTURE;
    
    /* Configure memory access patterns */
    adev->tx_ring.access_pattern = ACCESS_PATTERN_STREAMING;
    adev->rx_ring.access_pattern = ACCESS_PATTERN_SEQUENTIAL;
}
