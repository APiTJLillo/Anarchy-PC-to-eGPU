#ifndef ANARCHY_DEVICE_H
#define ANARCHY_DEVICE_H

#include <linux/device.h>
#include <linux/pci.h>
#include <linux/thunderbolt.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include "forward.h"
#include "pcie_state.h"
#include "ring.h"
#include "power_mgmt.h"
#include "perf_monitor.h"

/* Device state */
enum anarchy_device_state {
    ANARCHY_DEVICE_STATE_UNINITIALIZED = 0,
    ANARCHY_DEVICE_STATE_INITIALIZING,
    ANARCHY_DEVICE_STATE_READY,
    ANARCHY_DEVICE_STATE_ERROR,
    ANARCHY_DEVICE_STATE_DISCONNECTED
};

/* Device flags */
#define ANARCHY_DEVICE_FLAG_INITIALIZED   (1 << 0)
#define ANARCHY_DEVICE_FLAG_CONNECTED     (1 << 1)
#define ANARCHY_DEVICE_FLAG_ERROR         (1 << 2)
#define ANARCHY_DEVICE_FLAG_SUSPENDED     (1 << 3)

/* Main device structure */
struct anarchy_device {
    /* Device identification */
    struct device dev;  /* Note: Changed from pointer to embedded struct */
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
    
    /* Configuration */
    unsigned int dma_channels;   /* Number of DMA channels */
    unsigned int ring_buffer_size; /* Ring buffer size in bytes */
    
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
};

/* Device management functions */
int anarchy_device_init(struct anarchy_device *adev);
void anarchy_device_exit(struct anarchy_device *adev);
int anarchy_device_connect(struct anarchy_device *adev);
void anarchy_device_disconnect(struct anarchy_device *adev);
int anarchy_device_suspend(struct anarchy_device *adev);
int anarchy_device_resume(struct anarchy_device *adev);

#endif /* ANARCHY_DEVICE_H */
