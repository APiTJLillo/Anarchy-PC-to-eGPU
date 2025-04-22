#ifndef ANARCHY_DEVICE_H
#define ANARCHY_DEVICE_H

#include <linux/device.h>
#include <linux/thunderbolt.h>
#include <linux/workqueue.h>
#include "forward.h"
#include "gpu_config.h"
#include "pcie_state.h"
#include "ring.h"
#include "bandwidth.h"
#include "power_mgmt.h"
#include "perf_monitor.h"

/* Device configuration */
struct anarchy_device {
    /* Base device structure */
    struct device dev;
    struct tb_service *service;
    struct tb_switch *tb_switch;
    struct tb_port *tb_port;
    struct pci_dev *nhi;
    
    /* Core configuration */
    struct anarchy_pcie_state pcie;
    struct gpu_config gpu_cfg;
    struct power_profile power;
    struct bandwidth_config bw;
    
    /* Device capabilities */
    unsigned int max_speed;
    unsigned int max_lanes;
    unsigned int dma_channels;
    bool is_endpoint;
    bool is_initialized;
    
    /* Power management */
    unsigned int power_limit;
    unsigned int current_power;
    unsigned int fan_speed;
    
    /* Memory configuration */ 
    unsigned long vram_size;
    unsigned long mmio_size;
    void __iomem *mmio_base;
    
    /* Device identification */
    unsigned int vendor_id;
    unsigned int device_id;
    unsigned int subsys_vendor;
    unsigned int subsys_device;
    
    /* DMA/Ring configuration */
    struct anarchy_ring *rings;
    unsigned int num_rings;
    unsigned int ring_size;
    
    /* Performance monitoring */
    struct perf_monitor perf_monitor;
    struct perf_stats current_stats;
};

#endif /* ANARCHY_DEVICE_H */
    
    /* Performance monitoring */
    struct perf_monitor perf_monitor;
    struct anarchy_ring tx_ring;
    struct anarchy_ring rx_ring;
    unsigned int ring_buffer_size;
    unsigned int max_payload_size;
    unsigned int completion_timeout;
    
    /* Error handling */
    atomic_t error_count;
    struct workqueue_struct *wq;
    struct work_struct recovery_work;
    
    /* USB4/TB4 configuration */
    struct usb4_device_info *usb4;
    unsigned int link_width;
    unsigned int link_speed;
    
    /* Performance monitoring */
    struct perf_monitor *perf;
    unsigned int gpu_util;
    unsigned int mem_util;
    unsigned int pcie_util;
    
    /* Game compatibility */
    struct game_compat_layer *compat;
    unsigned int compat_flags;
    unsigned int drm_flags;
};

/* Device initialization */
int anarchy_device_init(struct anarchy_device *adev);
void anarchy_device_cleanup(struct anarchy_device *adev);

/* Power management */
int anarchy_device_suspend(struct anarchy_device *adev);
int anarchy_device_resume(struct anarchy_device *adev);

/* Error handling */
void anarchy_device_error(struct anarchy_device *adev, int error);

#endif /* ANARCHY_DEVICE_H */
