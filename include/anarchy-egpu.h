#ifndef ANARCHY_EGPU_H
#define ANARCHY_EGPU_H

#include <linux/types.h>
#include <linux/uuid.h>
#include <linux/thunderbolt.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>

#include "anarchy-pcie.h"

/* Forward declarations */
struct tb_xdomain;
struct tb_switch;
struct tb_service;
struct tb_nhi;
struct anarchy_frame;
struct anarchy_transfer;

#include "anarchy-device.h"

/* Constants and configuration */
#define ANARCHY_CFG_SIZE 4096
#define MMIO_SIZE (256 * 1024 * 1024)  /* 256MB MMIO space */
#define FB_SIZE (16ULL * 1024 * 1024 * 1024) /* 16GB framebuffer */

/* PCIe Capability Offsets */
#define ANARCHY_CAP_PM   0x40
#define ANARCHY_CAP_MSI  0x50
#define ANARCHY_CAP_PCIE 0x60

/* Device IDs */
#define NVIDIA_VENDOR_ID    0x10DE
#define RTX_4090_DEVICE_ID 0x2684

/* Categories and flags */
#define ANARCHY_CAT_DMA      BIT(3)  /* DMA operations */
#define ANARCHY_CAT_MEM      BIT(5)  /* Memory management */
#define ANARCHY_CAT_PCIE     BIT(2)  /* PCIe operations */
#define PCI_EXP_LNKSTA_DLL  0x2000  /* Data Link Layer Link Active */

/* Service protocol definitions */
#define ANARCHY_SERVICE_PROTOCOL_KEY 0x1
#define ANARCHY_SERVICE_PROTOCOL_ID  0x1
#define ANARCHY_PROTOCOL_VERSION     1
#define ANARCHY_DRIVER_VERSION       "1.0"

/* Service types and flags */
#define TB_SERVICE_EGPU        0x1
#define TB_SERVICE_AUTHORIZED  0x2

/* Ring configuration */
#define ANARCHY_NUM_BUFFERS 4
#define ANARCHY_BUFFER_SIZE 4096
#define ANARCHY_RING_SIZE 4096  /* 4KB ring buffer */
#define ANARCHY_DMA_ALIGN 4096

/* Additional enumerations */

enum anarchy_connection_state {
    ANARCHY_CONNECTION_STATE_DISCONNECTED = 0,
    ANARCHY_CONNECTION_STATE_CONNECTING,
    ANARCHY_CONNECTION_STATE_CONNECTED,
    ANARCHY_CONNECTION_STATE_DISCONNECTING
};

enum anarchy_error {
    ANARCHY_ERROR_NONE = 0,
    ANARCHY_ERROR_TIMEOUT,
    ANARCHY_ERROR_RESET,
    ANARCHY_ERROR_INVALID_STATE,
    ANARCHY_ERROR_INVALID_PARAM,
    ANARCHY_ERROR_NO_MEMORY,
    ANARCHY_ERROR_IO,
    ANARCHY_ERROR_DEVICE
};

/* Transfer structure */
struct anarchy_transfer {
    void *buffer;                /* User buffer */
    size_t size;                /* Transfer size */
    u32 flags;                  /* Transfer flags */
    int error;                  /* Transfer error code */
};

/* Function declarations */
/* Ring management */
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                          struct anarchy_transfer *transfer);

/* Thunderbolt management */
int anarchy_tb_init(void);
void anarchy_tb_exit(void);

/* GPU management */
int anarchy_gpu_power_on(struct anarchy_device *adev);
void anarchy_gpu_power_off(struct anarchy_device *adev);
int anarchy_gpu_init_error_handling(struct anarchy_device *adev);
void anarchy_gpu_cleanup_error_handling(struct anarchy_device *adev);

#endif /* ANARCHY_EGPU_H */
