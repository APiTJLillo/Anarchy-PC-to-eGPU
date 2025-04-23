#include <linux/module.h>
#include <linux/slab.h>
#include "include/anarchy_device.h"
#include "include/dma.h"
#include "include/dma_types.h"

/* DMA Register definitions */
#define DMA_REG_BASE          0x10000
#define DMA_CHANNEL_OFFSET    0x100
#define DMA_PRIO_OFFSET       0x20

/* Calculate register offset for DMA priority setting */
#define DMA_PRIO_REG(channel) (DMA_REG_BASE + (channel * DMA_CHANNEL_OFFSET) + DMA_PRIO_OFFSET)

/* Priority level values */
#define DMA_PRIO_LEVEL_NORMAL    0x1
#define DMA_PRIO_LEVEL_TEXTURE   0x2
#define DMA_PRIO_LEVEL_HIGH      0x3

/* Basic DMA transfer */
dma_addr_t anarchy_dma_transfer(struct anarchy_device *adev, void *data, size_t size)
{
    dma_addr_t dma_addr;
    int ret;

    /* Map the data for DMA */
    dma_addr = dma_map_single(&adev->pdev->dev, data, size, DMA_TO_DEVICE);
    if (dma_mapping_error(&adev->pdev->dev, dma_addr))
        return 0;

    /* Start the transfer using device-specific function */
    ret = anarchy_dma_device_start_transfer(adev, 0, dma_addr, 0, size);
    if (ret) {
        dma_unmap_single(&adev->pdev->dev, dma_addr, size, DMA_TO_DEVICE);
        return 0;
    }

    return dma_addr;
}

/* Priority-based DMA transfer */
int anarchy_dma_transfer_priority(struct anarchy_device *adev, void *data,
                                size_t size, enum anarchy_dma_priority priority)
{
    int channel = 0;
    int ret;

    /* Select channel based on priority */
    switch (priority) {
    case ANARCHY_DMA_PRIO_HIGH:
        channel = 1;
        break;
    case ANARCHY_DMA_PRIO_TEXTURE:
        channel = 2;
        break;
    case ANARCHY_DMA_PRIO_NORMAL:
        channel = 0;
        break;
    default:
        return -EINVAL;
    }

    /* Set channel priority */
    ret = anarchy_dma_set_device_priority(adev, channel, priority);
    if (ret)
        return ret;

    /* Perform the transfer */
    if (!anarchy_dma_transfer(adev, data, size))
        return -EIO;

    return 0;
}

/* DMA cleanup */
void anarchy_dma_cleanup(struct anarchy_device *adev, dma_addr_t dma_addr, size_t size)
{
    if (dma_addr)
        dma_unmap_single(&adev->pdev->dev, dma_addr, size, DMA_TO_DEVICE);
}

int anarchy_dma_set_device_priority(struct anarchy_device *adev, int channel,
                                  enum anarchy_dma_priority priority)
{
    u32 prio_val;

    if (!adev || channel < 0 || channel >= adev->dma_channels)
        return -EINVAL;

    /* Set the priority for the specified channel */
    switch (priority) {
    case ANARCHY_DMA_PRIO_HIGH:
        prio_val = DMA_PRIO_LEVEL_HIGH;
        break;
    case ANARCHY_DMA_PRIO_TEXTURE:
        prio_val = DMA_PRIO_LEVEL_TEXTURE;
        break;
    case ANARCHY_DMA_PRIO_NORMAL:
        prio_val = DMA_PRIO_LEVEL_NORMAL;
        break;
    default:
        return -EINVAL;
    }

    /* Write priority value to the register */
    writel(prio_val, adev->mmio_base + DMA_PRIO_REG(channel));

    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_dma_set_device_priority);

EXPORT_SYMBOL_GPL(anarchy_dma_transfer);
EXPORT_SYMBOL_GPL(anarchy_dma_transfer_priority);
EXPORT_SYMBOL_GPL(anarchy_dma_cleanup);
