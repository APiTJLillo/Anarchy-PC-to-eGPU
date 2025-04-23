#ifndef ANARCHY_DEVICE_H
#define ANARCHY_DEVICE_H

#include <linux/device.h>
#include <linux/pci.h>
#include <linux/thunderbolt.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include "anarchy_device_forward.h"
#include "pcie_forward.h"
#include "pcie_types.h"
#include "ring.h"
#include "power_mgmt.h"
#include "perf_monitor.h"
#include "thermal_forward.h"
#include "gpu_emu_forward.h"
#include "bandwidth.h"
#include "bandwidth_config.h"

/* Main device structure */
struct anarchy_device {
    /* Device identification */
    struct device *dev;  /* Changed back to pointer */
    struct pci_dev *pdev;
    char name[32];
    u32 device_id;
    u32 vendor_id;
    
    /* Device state */
    enum anarchy_device_state state;
    u32 flags;
    struct mutex lock;
    
    /* Thunderbolt/PCIe state */
    struct anarchy_pcie_state pcie_state;
    struct tb_service *service;
    struct tb_port *tb_port;  /* Added TB port */
    
    /* Configuration */
    unsigned int dma_channels;   /* Number of DMA channels */
    unsigned int ring_buffer_size; /* Ring buffer size in bytes */
    
    /* DMA configuration */
    u32 dma_batch_size;
    bool low_latency_mode;
    
    /* Ring buffers */
    struct anarchy_ring tx_ring;
    struct anarchy_ring rx_ring;
    
    /* Command processing */
    struct command_processor *cmd_proc;
    
    /* Game compatibility */
    struct game_compat_layer *compat_layer;
    
    /* Performance monitoring */
    struct perf_monitor perf_monitor;
    
    /* Power management */
    struct power_profile power_profile;
    
    /* Bandwidth monitoring */
    struct bandwidth_config bandwidth;
    bool texture_compression_enabled;
    
    /* Workqueues */
    struct workqueue_struct *wq;
    struct work_struct init_work;
    struct work_struct cleanup_work;
    
    /* Memory mapping */
    void __iomem *mmio_base;    /* PCIe MMIO base address */
    resource_size_t mmio_size;   /* PCIe MMIO region size */
    
    /* Statistics */
    atomic_t ref_count;
    
    /* Private data */
    void *private_data;

    struct thermal_profile thermal_profile;

    /* GPU Emulation */
    struct gpu_emu_interface *gpu_emu;
};

/* Device management functions */
int anarchy_device_init(struct anarchy_device *adev);
void anarchy_device_exit(struct anarchy_device *adev);
int anarchy_device_connect(struct anarchy_device *adev);
void anarchy_device_disconnect(struct anarchy_device *adev);
int anarchy_device_suspend(struct anarchy_device *adev);
int anarchy_device_resume(struct anarchy_device *adev);

#endif /* ANARCHY_DEVICE_H */
