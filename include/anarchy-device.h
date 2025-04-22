#ifndef ANARCHY_DEVICE_H
#define ANARCHY_DEVICE_H

#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>
#include <linux/pci.h>
#include <linux/wait.h>
#include <linux/dma-mapping.h>
#include <linux/thunderbolt.h>

/* Forward declarations */
struct tb_service;
struct pci_dev;
struct tb_ring;
struct anarchy_frame;
struct anarchy_transfer;
struct anarchy_device;
struct anarchy_pcie_config;

/* Enums needed by device structures */
enum anarchy_ring_state {
    ANARCHY_RING_STATE_STOPPED = 0,
    ANARCHY_RING_STATE_STARTING,
    ANARCHY_RING_STATE_RUNNING,
    ANARCHY_RING_STATE_STOPPING
};

enum anarchy_pcie_link_state {
    ANARCHY_PCIE_STATE_UNKNOWN = 0,
    ANARCHY_PCIE_STATE_NORMAL,
    ANARCHY_PCIE_STATE_LINK_DOWN,
    ANARCHY_PCIE_STATE_TRAINING,
    ANARCHY_PCIE_STATE_ERROR,
    ANARCHY_PCIE_STATE_RECOVERY,
    ANARCHY_PCIE_STATE_DISABLED
};

enum anarchy_pcie_error_type {
    ANARCHY_PCIE_ERROR_NONE = 0,
    ANARCHY_PCIE_ERROR_LINK_DOWN,
    ANARCHY_PCIE_ERROR_TRAINING,
    ANARCHY_PCIE_ERROR_THUNDERBOLT,
    ANARCHY_PCIE_ERROR_TIMEOUT,
    ANARCHY_PCIE_ERROR_INVALID
};

/* Frame structures */
struct anarchy_frame {
    struct ring_frame frame;     /* Must be first */
    void *buffer;                /* DMA buffer */
    dma_addr_t buffer_phy;       /* Physical address of DMA buffer */
    size_t size;                 /* Buffer size */
    u32 flags;                   /* Frame flags */
    void *priv;                  /* Private data */
};

/* Ring structure */
struct anarchy_ring {
    struct tb_ring *ring;        /* Thunderbolt ring */
    struct ring_frame frame;     /* Current frame */
    struct anarchy_frame frames[4];
    struct anarchy_frame *current_frame;
    enum anarchy_ring_state state;
    atomic_t error_count;
    atomic_t pending;
    struct anarchy_transfer *current_transfer;
    spinlock_t lock;
    wait_queue_head_t wait;
    struct anarchy_device *adev;
    int id;                      /* Ring ID */
    void *priv;                  /* Private data */
};

/* PCIe state structure */
struct anarchy_pcie_state {
    struct pci_dev *nhi;
    enum anarchy_pcie_link_state state;
    struct anarchy_pcie_config config;
    struct work_struct recovery_work;
    spinlock_t recovery_lock;
    atomic_t retries;
    unsigned long last_recovery;
    unsigned long last_state_change;
    enum anarchy_pcie_error_type error;
};

/* Main device structure */
struct anarchy_device {
    struct device dev;           /* Must be first */
    struct tb_service *service;
    struct pci_dev *nhi;
    struct anarchy_ring tx_ring;
    struct anarchy_ring rx_ring;
    struct workqueue_struct *wq;
    struct work_struct recovery_work;
    atomic_t error_count;
    u32 max_speed;              /* PCIe speed */
    u32 max_lanes;              /* PCIe lanes */
    void *priv;                 /* Private data */
    struct anarchy_pcie_state pcie;  /* PCIe state structure - must be last */
};

/* Helper function to get parent anarchy_device */
static inline struct anarchy_device *to_anarchy_device(struct anarchy_pcie_state *pcie)
{
    return container_of(pcie, struct anarchy_device, pcie);
}

#endif /* ANARCHY_DEVICE_H */
