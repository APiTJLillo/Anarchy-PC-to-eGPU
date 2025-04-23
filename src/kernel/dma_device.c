#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/iopoll.h>
#include "include/anarchy_device.h"
#include "include/dma.h"
#include "include/dma_types.h"

/* DMA device-specific registers */
#define DMA_DEV_CTRL_REG     0x20000
#define DMA_DEV_STATUS_REG   0x20004
#define DMA_DEV_ADDR_REG     0x20008
#define DMA_DEV_SIZE_REG     0x2000C

/* DMA control bits */
#define DMA_CTRL_START       BIT(0)
#define DMA_CTRL_COMPLETE    BIT(1)
#define DMA_CTRL_ERROR       BIT(2)

int anarchy_dma_device_start_transfer(struct anarchy_device *adev, int channel,
                                    dma_addr_t addr, u32 offset, size_t size)
{
    u32 status;
    int ret = 0;

    if (!adev || channel < 0 || channel >= adev->dma_channels)
        return -EINVAL;

    /* Write DMA address and size */
    writel(addr + offset, adev->mmio_base + DMA_DEV_ADDR_REG);
    writel(size, adev->mmio_base + DMA_DEV_SIZE_REG);

    /* Start the transfer */
    writel(DMA_CTRL_START, adev->mmio_base + DMA_DEV_CTRL_REG);

    /* Wait for completion or error */
    ret = readl_poll_timeout(adev->mmio_base + DMA_DEV_STATUS_REG,
                           status,
                           (status & (DMA_CTRL_COMPLETE | DMA_CTRL_ERROR)),
                           1000, 1000000);

    if (ret)
        return ret;

    if (status & DMA_CTRL_ERROR)
        return -EIO;

    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_dma_device_start_transfer); 