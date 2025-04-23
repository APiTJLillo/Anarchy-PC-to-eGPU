#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/thunderbolt.h>
#include "include/anarchy_device.h"
#include "include/common.h"
#include "include/pcie_types.h"
#include "include/perf_monitor.h"
#include "include/dma.h"
#include "include/thunderbolt_utils.h"

/* PCIe bandwidth counters */
#define PCIE_RX_COUNTER      0x5000
#define PCIE_TX_COUNTER      0x5004

/* Bandwidth monitoring interval (in milliseconds) */
#define BANDWIDTH_UPDATE_INTERVAL 1000

/* TB4/USB4 bandwidth configuration */
#define TB4_MAX_BANDWIDTH    40000  /* 40 Gbps max */
#define PCIE_X8_BANDWIDTH    16000  /* ~16 GB/s for PCIe 4.0 x8 */
#define MIN_GAMING_BANDWIDTH 10000  /* 10 Gbps minimum for gaming */

/* Get current PCIe bandwidth usage */
u32 anarchy_pcie_get_bandwidth_usage(struct anarchy_device *adev)
{
    struct bandwidth_config *bw = &adev->bandwidth;
    unsigned long flags;
    u32 usage;
    
    spin_lock_irqsave(&bw->lock, flags);
    usage = bw->current_bandwidth;
    spin_unlock_irqrestore(&bw->lock, flags);
    
    return usage;
}

static void bandwidth_update_work(struct work_struct *work)
{
    struct delayed_work *delayed_work = to_delayed_work(work);
    struct bandwidth_config *bw = container_of(delayed_work,
                                             struct bandwidth_config,
                                             update_work);
    struct anarchy_device *adev = container_of(bw, struct anarchy_device,
                                             bandwidth);
    unsigned long flags;
    u32 rx_bandwidth, tx_bandwidth;
    
    /* Read PCIe counters */
    rx_bandwidth = readl(adev->mmio_base + PCIE_RX_COUNTER);
    tx_bandwidth = readl(adev->mmio_base + PCIE_TX_COUNTER);
    
    spin_lock_irqsave(&bw->lock, flags);
    bw->current_bandwidth = rx_bandwidth + tx_bandwidth;
    bw->available_bandwidth = tb_port_get_bandwidth(adev->tb_port);
    
    if (bw->available_bandwidth < MIN_GAMING_BANDWIDTH) {
        if (!bw->bandwidth_critical) {
            dev_warn(adev->dev, "Low bandwidth detected (%d Gbps)\n",
                    bw->available_bandwidth / 1000);
            bw->bandwidth_critical = true;
        }
        
        /* Apply optimizations */
        anarchy_dma_optimize_transfers(adev);
        if (!adev->texture_compression_enabled) {
            dev_info(adev->dev, "Enabling texture compression\n");
            adev->texture_compression_enabled = true;
        }
    } else if (bw->bandwidth_critical && 
               bw->available_bandwidth >= MIN_GAMING_BANDWIDTH) {
        dev_info(adev->dev, "Bandwidth restored to normal levels\n");
        bw->bandwidth_critical = false;
    }
    
    bw->last_update = jiffies;
    spin_unlock_irqrestore(&bw->lock, flags);
    
    /* Schedule next update */
    schedule_delayed_work(&bw->update_work, 
                         msecs_to_jiffies(BANDWIDTH_UPDATE_INTERVAL));
}

/* Initialize bandwidth monitoring */
int init_bandwidth_monitoring(struct anarchy_device *adev)
{
    struct bandwidth_config *bw = &adev->bandwidth;
    
    /* Initialize bandwidth monitoring state */
    bw->required_bandwidth = PCIE_X8_BANDWIDTH;
    bw->current_bandwidth = 0;
    bw->available_bandwidth = TB4_MAX_BANDWIDTH;
    bw->bandwidth_critical = false;
    
    /* Initialize synchronization */
    spin_lock_init(&bw->lock);
    
    /* Initialize work queue */
    INIT_DELAYED_WORK(&bw->update_work, bandwidth_update_work);
    
    /* Start monitoring */
    schedule_delayed_work(&bw->update_work,
                         msecs_to_jiffies(BANDWIDTH_UPDATE_INTERVAL));
    
    return 0;
}

/* Cleanup bandwidth monitoring */
void cleanup_bandwidth_monitoring(struct anarchy_device *adev)
{
    struct bandwidth_config *bw = &adev->bandwidth;
    
    cancel_delayed_work_sync(&bw->update_work);
}

EXPORT_SYMBOL_GPL(anarchy_pcie_get_bandwidth_usage);
EXPORT_SYMBOL_GPL(init_bandwidth_monitoring);
EXPORT_SYMBOL_GPL(cleanup_bandwidth_monitoring);
