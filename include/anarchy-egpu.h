#ifndef ANARCHY_EGPU_H
#define ANARCHY_EGPU_H

#include <linux/uuid.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/thunderbolt.h>
#include <linux/atomic.h>

#define ANARCHY_PROTOCOL_VERSION 1

/* PCIe Configuration */
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

/* PCIe Categories - must match debug.h */
#define ANARCHY_CAT_DMA      BIT(3)  /* DMA operations */
#define ANARCHY_CAT_MEM      BIT(5)  /* Memory management */
#define ANARCHY_CAT_PCIE     BIT(2)  /* PCIe operations */

/* PCIe Link Status */
#define PCI_EXP_LNKSTA_DLL  0x2000  /* Data Link Layer Link Active */

/* PCIe Configuration Space */
struct anarchy_pcie_caps {
    u8 pm_cap;
    u16 pm_ctrl;
    u8 msi_cap;
    u16 msi_ctrl;
    u64 msi_addr;
    u16 msi_data;
    u8 pcie_cap;
    u16 pcie_ctrl;
    u32 link_cap;
    u16 link_ctrl;
    u16 link_status;
};

struct anarchy_pcie_config {
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8 revision;
    u8 header_type;
    u8 cache_line_size;
    u8 latency_timer;
    u8 bist;
    u32 bar[6];
    u32 bar_mask[6];
    u32 rom_base;
    bool rom_enabled;
    u8 interrupt_line;
    u8 interrupt_pin;
    void __iomem *mmio_base;
    void __iomem *fb_base;
    struct anarchy_pcie_caps caps;
};

/* Statistics tracking */
struct anarchy_stats {
    atomic_t pcie_errors;
    atomic_t dma_errors;
    atomic_t tb_errors;
};

/* Service UUID - must match the one in thunderbolt.c */
static const uuid_t ANARCHY_SERVICE_UUID =
    UUID_INIT(0x12345678, 0x1234, 0x1234, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef);

#define ANARCHY_MAX_PCIE_LANES 16  // RTX 4090 supports up to 16 lanes
#define ANARCHY_RING_SIZE (64 * 1024)  // 64KB ring buffer
#define ANARCHY_NUM_BUFFERS 32
#define ANARCHY_DMA_SIZE_ALIGN 256

/* Maximum number of connection retries */
#define ANARCHY_MAX_RETRIES 3
#define ANARCHY_RETRY_DELAY_MS 1000

enum anarchy_ring_state {
    RING_STATE_STOPPED = 0,
    RING_STATE_RUNNING = 1,
    RING_STATE_ERROR = 2,
};

/* Connection states */
enum anarchy_conn_state {
    ANARCHY_CONN_DISCONNECTED = 0,
    ANARCHY_CONN_CONNECTING,
    ANARCHY_CONN_CONNECTED,
    ANARCHY_CONN_DISCONNECTING,
    ANARCHY_CONN_ERROR,
    ANARCHY_CONN_RECOVERY,
    ANARCHY_CONN_RECOVERY_IN_PROGRESS,  /* Add this new state */
};

enum anarchy_connection_error {
    ANARCHY_ERR_NONE = 0,
    ANARCHY_ERR_NO_DEVICE,
    ANARCHY_ERR_SERVICE_INIT,
    ANARCHY_ERR_RING_SETUP,
    ANARCHY_ERR_PCIE_INIT
};

/* Performance monitoring events */
enum anarchy_tb_event {
    ANARCHY_TB_ERROR = 0,
    ANARCHY_TB_TRANSFER,
    ANARCHY_TB_RECOVERY,
};

struct anarchy_frame {
    void *data;
    dma_addr_t dma;
    size_t size;
    u32 flags;
};

struct anarchy_transfer {
    void *data;
    size_t size;
    u32 flags;
    bool completed;
    int error;      /* Error code if transfer failed */
};

struct anarchy_ring {
    struct tb_ring *ring;
    struct anarchy_device *adev;
    spinlock_t lock;
    wait_queue_head_t wait;
    struct anarchy_frame frames[ANARCHY_NUM_BUFFERS];
    struct anarchy_transfer *current_transfer;
    atomic_t pending;
    enum anarchy_ring_state state;
    struct ring_frame frame;  /* Current frame being processed */
};

/* PCIe states */
enum tb_pcie_state {
    TB_PCIE_OK = 0,
    TB_PCIE_NORMAL = TB_PCIE_OK,  /* Alias for backward compatibility */
    TB_PCIE_LINK_DOWN,
    TB_PCIE_TRAINING,
    TB_PCIE_ERROR,
    TB_PCIE_POWER_CHANGE,
    TB_PCIE_MODE_SWITCHING       /* New state for TB/USB4 mode switching */
};

/* PCIe state management */
struct anarchy_pcie_state {
    struct mutex recovery_lock;
    enum tb_pcie_state state;
    atomic_t retries;
    unsigned long last_recovery;
};

struct anarchy_device {
    struct tb_service *service;
    struct tb_nhi *nhi;
    struct anarchy_ring tx_ring;
    struct anarchy_ring rx_ring;
    atomic_t conn_state;  /* Uses values from anarchy_connection_state */
    enum anarchy_connection_error conn_error;
    struct work_struct recovery_work;
    struct workqueue_struct *wq;
    atomic_t error_count;  /* Counter for connection errors */
    struct device *dev;  /* Parent device for logging */
    struct anarchy_pcie_config pcie_cfg;
    struct anarchy_pcie_state pcie;  /* Add PCIe state */
    struct anarchy_stats stats;
    u8 max_lanes;  /* Maximum supported PCIe lanes */
    u8 max_speed;  /* Maximum supported PCIe speed */
    wait_queue_head_t xdomain_wait;  /* Wait queue for XDomain connection */
};

/* PCIe error codes */
enum anarchy_pcie_error {
    ANARCHY_PCIE_ERR_NONE = 0,
    ANARCHY_PCIE_ERR_TRAINING_FAILED,
    ANARCHY_PCIE_ERR_LINK_DOWN,
    ANARCHY_PCIE_ERR_SPEED_DOWNGRADE,
    ANARCHY_PCIE_ERR_WIDTH_DOWNGRADE,
    ANARCHY_PCIE_ERR_TIMEOUT,
    ANARCHY_PCIE_ERR_DEVICE_NOT_FOUND,
    ANARCHY_PCIE_ERR_RETRY
};

/* PCIe recovery parameters */
#define ANARCHY_PCIE_MAX_RETRIES 5
#define ANARCHY_PCIE_RETRY_DELAY_MS 100

// Function declarations
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                          struct anarchy_transfer *transfer);

// Performance monitoring functions
void anarchy_perf_tb_event(enum anarchy_tb_event event, int value);

// Thunderbolt subsystem init/exit
int anarchy_tb_init(void);
void anarchy_tb_exit(void);

/* PCIe functions */
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);
int anarchy_pcie_setup_bars(struct anarchy_device *adev);
int anarchy_pcie_setup_capabilities(struct anarchy_device *adev);
int anarchy_pcie_read_config(struct anarchy_device *adev, int where, int size, u32 *val);
int anarchy_pcie_write_config(struct anarchy_device *adev, int where, int size, u32 val);
void anarchy_pcie_check_link(struct anarchy_device *adev);
void anarchy_pcie_retrain_link(struct anarchy_device *adev);
void anarchy_pcie_recovery_work(struct work_struct *work);
void anarchy_pcie_handle_error(struct anarchy_device *adev, int error);

/* Error handling functions */
void anarchy_handle_connection_error(struct anarchy_device *adev, int err);

/* Device access functions */
struct anarchy_device *anarchy_get_device(void);

#endif /* ANARCHY_EGPU_H */ 