#include <linux/module.h>
#include <linux/dma-mapping.h>
#include "include/anarchy_device.h"
#include "include/dma.h"
#include "include/command_types.h"

int anarchy_dma_transfer(struct anarchy_device *adev, void *data, size_t size)
{
    dma_addr_t dma_addr;
    int ret = 0;

    if (!adev || !data || !size)
        return -EINVAL;

    /* Map the data for DMA */
    dma_addr = dma_map_single(&adev->dev, data, size, DMA_TO_DEVICE);
    if (dma_mapping_error(&adev->dev, dma_addr)) {
        dev_err(&adev->dev, "Failed to map DMA buffer\n");
        return -ENOMEM;
    }

    /* TODO: Implement actual DMA transfer using PCIe registers */
    /* For now just simulate success */

    dma_unmap_single(&adev->dev, dma_addr, size, DMA_TO_DEVICE);
    return ret;
}

int anarchy_dma_transfer_priority(struct anarchy_device *adev, void *data,
                                size_t size, enum dma_priority priority)
{
    int channel;
    int ret;

    /* Select DMA channel based on priority */
    switch (priority) {
    case PRIORITY_HIGH:
        channel = 0;  /* Highest priority channel */
        break;
    case PRIORITY_TEXTURE:
        channel = 1;  /* Dedicated texture channel */
        break;
    case PRIORITY_NORMAL:
        channel = 2;  /* Normal priority channel */
        break;
    default:
        channel = 3;  /* Low priority channel */
        break;
    }

    /* Set channel priority */
    ret = anarchy_dma_set_channel_priority(adev, channel, priority);
    if (ret)
        return ret;

    /* Perform transfer */
    return anarchy_dma_transfer(adev, data, size);
}

int anarchy_dma_set_channel_priority(struct anarchy_device *adev, int channel,
                                   enum dma_priority priority)
{
    if (!adev || channel >= adev->dma_channels)
        return -EINVAL;

    /* TODO: Implement priority configuration in hardware */
    return 0;
}

EXPORT_SYMBOL_GPL(anarchy_dma_transfer);
EXPORT_SYMBOL_GPL(anarchy_dma_transfer_priority);
EXPORT_SYMBOL_GPL(anarchy_dma_set_channel_priority);
