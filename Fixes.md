# GPU Passthrough Implementation Guide

This guide provides step-by-step instructions for implementing the GPU passthrough system that allows your Linux machine (Lenovo P16 with RTX 4090) to present itself as an external GPU to your Windows machine (Legion Go) over USB4/Thunderbolt.

## Table of Contents

1. [Fixing Compilation Errors](#1-fixing-compilation-errors)
2. [Implementing Core Components](#2-implementing-core-components)
3. [Testing and Debugging](#3-testing-and-debugging)
4. [Performance Optimization](#4-performance-optimization)
5. [Troubleshooting](#5-troubleshooting)

## 1. Fixing Compilation Errors

The first step is to fix the compilation errors in your current codebase. These errors are primarily related to Linux kernel API compatibility issues.

### 1.1. Struct Redefinition Fixes

Several structures in your code are being redefined, causing compilation errors. Here's how to fix them:

#### Fix `forward.h`

Replace the content of `forward.h` with:

```c
#ifndef ANARCHY_FORWARD_H
#define ANARCHY_FORWARD_H

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/completion.h>

/* Forward declarations only - no struct definitions */
struct anarchy_device;
struct pci_dev;
struct device;
struct tb_service;
struct tb_ring;

/* Use Linux kernel's ring_frame instead of redefining it */
/* struct ring_frame is already defined in linux/thunderbolt.h */

/* Forward declarations for our custom structures */
struct perf_state;
struct perf_monitor;
struct gpu_config;
struct gpu_config_ops;
struct power_profile;
struct anarchy_ring;
struct anarchy_transfer;
struct anarchy_pcie_state;
struct tb_service_config;
struct usb4_device_info;
struct game_compat_layer;
struct game_profile;
struct command_processor;
struct bandwidth_config;
struct thermal_profile;
struct gpu_emu_config;
struct gpu_emu_interface;
struct game_memory_region;

#endif /* ANARCHY_FORWARD_H */
```

#### Fix `pcie_types.h` and `pcie_state.h`

These files have conflicting enum definitions. Update them as follows:

**pcie_types.h**:
```c
#ifndef ANARCHY_PCIE_TYPES_H
#define ANARCHY_PCIE_TYPES_H

#include <linux/types.h>

/* PCIe link speed definitions */
enum anarchy_pcie_speed {
    ANARCHY_PCIE_GEN1 = 1, /* 2.5 GT/s */
    ANARCHY_PCIE_GEN2 = 2, /* 5.0 GT/s */
    ANARCHY_PCIE_GEN3 = 3, /* 8.0 GT/s */
    ANARCHY_PCIE_GEN4 = 4, /* 16.0 GT/s */
    ANARCHY_PCIE_GEN5 = 5  /* 32.0 GT/s */
};

/* PCIe link width definitions */
enum anarchy_pcie_width {
    ANARCHY_PCIE_x1 = 1,
    ANARCHY_PCIE_x2 = 2,
    ANARCHY_PCIE_x4 = 4,
    ANARCHY_PCIE_x8 = 8,
    ANARCHY_PCIE_x16 = 16
};

/* PCIe error codes - renamed to avoid conflict */
enum anarchy_pcie_error_code {
    ANARCHY_PCIE_ERROR_NONE = 0,
    ANARCHY_PCIE_ERROR_LINK_DOWN = -1,
    ANARCHY_PCIE_ERROR_TRAINING = -2,
    ANARCHY_PCIE_ERROR_SPEED = -3,
    ANARCHY_PCIE_ERROR_WIDTH = -4,
    ANARCHY_PCIE_ERROR_TIMEOUT = -5
};

/* PCIe configuration space access modes */
enum anarchy_pcie_access {
    ANARCHY_PCIE_ACCESS_MMIO = 0,
    ANARCHY_PCIE_ACCESS_CONFIG = 1
};

/* PCIe capabilities */
struct anarchy_pcie_caps {
    enum anarchy_pcie_speed max_speed;
    enum anarchy_pcie_width max_width;
    bool ats_support;
    bool sriov_support;
    bool aspm_support;
    u32 max_payload_size;
    u32 max_read_request_size;
};

#endif /* ANARCHY_PCIE_TYPES_H */
```

**pcie_state.h**:
```c
#ifndef ANARCHY_PCIE_STATE_H
#define ANARCHY_PCIE_STATE_H

#include <linux/pci.h>
#include <linux/workqueue.h>
#include "forward.h"
#include "pcie_types.h"

/* PCIe link states */
enum anarchy_pcie_link_state {
    ANARCHY_PCIE_STATE_UNKNOWN = 0,
    ANARCHY_PCIE_STATE_NORMAL,
    ANARCHY_PCIE_STATE_LINK_DOWN,
    ANARCHY_PCIE_STATE_ERROR,
    ANARCHY_PCIE_STATE_RECOVERY,
    ANARCHY_PCIE_STATE_TRAINING
};

/* PCIe error types - renamed to avoid conflict with pcie_types.h */
enum anarchy_pcie_error_type {
    ANARCHY_PCIE_ERRTYPE_NONE = 0,
    ANARCHY_PCIE_ERRTYPE_LINK_DOWN = 1,
    ANARCHY_PCIE_ERRTYPE_TRAINING = 2,
    ANARCHY_PCIE_ERRTYPE_SPEED = 3,
    ANARCHY_PCIE_ERRTYPE_WIDTH = 4,
    ANARCHY_PCIE_ERRTYPE_TIMEOUT = 5
};

/* PCIe state tracking */
struct anarchy_pcie_state {
    /* Hardware state */
    struct pci_dev *nhi;
    enum anarchy_pcie_link_state state;
    enum anarchy_pcie_speed link_speed;
    enum anarchy_pcie_width link_width;
    
    /* Error handling */
    spinlock_t recovery_lock;
    atomic_t retries;
    unsigned long last_recovery;
    struct work_struct recovery_work;
    
    /* Back pointer to parent device */
    struct anarchy_device *adev;
    
    /* Stats tracking */
    u32 training_attempts;
    u32 successful_trains;
    u64 total_uptime;
    unsigned long last_up;
    unsigned long last_down;
};

/* PCIe state management */
int anarchy_pcie_init(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);
void anarchy_pcie_disable(struct anarchy_device *adev);
int anarchy_pcie_train_link(struct anarchy_device *adev);
void anarchy_pcie_handle_error(struct anarchy_device *adev, enum anarchy_pcie_error_type error);

#endif /* ANARCHY_PCIE_STATE_H */
```

#### Fix `ring.h`

Update `ring.h` to resolve function declaration conflicts:

```c
#ifndef ANARCHY_RING_H
#define ANARCHY_RING_H

#include <linux/types.h>
#include <linux/thunderbolt.h>
#include "forward.h"

/* Ring states */
enum anarchy_ring_state {
    ANARCHY_RING_STATE_STOPPED = 0,
    ANARCHY_RING_STATE_RUNNING,
    ANARCHY_RING_STATE_ERROR
};

/* Transfer configuration */
struct anarchy_transfer {
    void *buffer;
    size_t size;
    u32 flags;
    int error;
    void *context;
    void (*callback)(void *context, int error);
};

/* Ring configuration */
struct anarchy_ring {
    struct anarchy_device *adev;
    struct tb_ring *ring;
    
    /* State tracking */
    enum anarchy_ring_state state;
    
    /* Ring buffer */
    void *buffer;
    size_t buffer_size;
    u32 head;
    u32 tail;
    
    /* Transfer queue */
    struct anarchy_transfer *transfers;
    unsigned int num_transfers;
    spinlock_t lock;
    
    /* Statistics */
    atomic_t bytes_transferred;
    atomic_t transfer_errors;
    atomic_t error_count;
    atomic_t pending;
    
    /* Wait queue */
    wait_queue_head_t wait;
    
    /* Transfer state */
    struct anarchy_transfer *current_transfer;
    /* Use Linux kernel's ring_frame instead of our own */
    struct ring_frame frame;
    bool is_tx;
    
    /* DMA buffers */
    struct {
        void *buffer;
        dma_addr_t buffer_phy;
        size_t size;
        u32 flags;
    } frames[32];
};

/* Ring lifecycle - consistent function signatures */
int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring);

/* Ring operations - internal interface */
void anarchy_ring_error(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring, 
                         struct anarchy_transfer *transfer);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring, 
                          struct anarchy_transfer *transfer);

#endif /* ANARCHY_RING_H */
```

#### Fix `power_mgmt.h` and `perf_monitor.h`

Update these files to avoid struct redefinition:

**power_mgmt.h**:
```c
#ifndef ANARCHY_POWER_MGMT_H
#define ANARCHY_POWER_MGMT_H

#include <linux/types.h>
#include "forward.h"

/* Only define power_profile if it hasn't been defined yet */
#ifndef ANARCHY_POWER_PROFILE_DEFINED
#define ANARCHY_POWER_PROFILE_DEFINED

struct power_profile {
    unsigned int fan_speed;     /* Current fan speed percentage */
    unsigned int power_limit;   /* Current power limit in watts */
    bool dynamic_control;       /* Whether dynamic control is enabled */
};

#endif /* ANARCHY_POWER_PROFILE_DEFINED */

/* Power management functions */
int anarchy_power_init(struct anarchy_device *adev);
void anarchy_power_exit(struct anarchy_device *adev);
int anarchy_power_set_profile(struct anarchy_device *adev, struct power_profile *profile);
int anarchy_power_get_profile(struct anarchy_device *adev, struct power_profile *profile);
int anarchy_power_set_fan_speed(struct anarchy_device *adev, unsigned int speed);
int anarchy_power_set_power_limit(struct anarchy_device *adev, unsigned int limit);

#endif /* ANARCHY_POWER_MGM_H */
```

**perf_monitor.h**:
```c
#ifndef ANARCHY_PERF_MONITOR_H
#define ANARCHY_PERF_MONITOR_H

#include <linux/types.h>
#include <linux/workqueue.h>
#include "forward.h"

/* Only define perf_state if it hasn't been defined yet */
#ifndef ANARCHY_PERF_STATE_DEFINED
#define ANARCHY_PERF_STATE_DEFINED

struct perf_state {
    unsigned int gpu_clock;
    unsigned int mem_clock;
    unsigned int power_draw;
    unsigned int temperature;
    unsigned int fan_speed;
    unsigned int gpu_util;
    unsigned int mem_util;
    unsigned int pcie_util;
    unsigned int vram_used;
};

#endif /* ANARCHY_PERF_STATE_DEFINED */

/* Only define perf_monitor if it hasn't been defined yet */
#ifndef ANARCHY_PERF_MONITOR_DEFINED
#define ANARCHY_PERF_MONITOR_DEFINED

struct perf_monitor {
    struct anarchy_device *adev;
    struct perf_state current_state;
    struct workqueue_struct *wq;
    struct delayed_work update_work;
    spinlock_t lock;
    unsigned int update_interval;
    bool enabled;
};

#endif /* ANARCHY_PERF_MONITOR_DEFINED */

/* Performance monitoring functions */
int anarchy_perf_init(struct anarchy_device *adev);
void anarchy_perf_exit(struct anarchy_device *adev);
int anarchy_perf_start(struct anarchy_device *adev);
void anarchy_perf_stop(struct anarchy_device *adev);
int anarchy_perf_get_state(struct anarchy_device *adev, struct perf_state *state);

#endif /* ANARCHY_PERF_MONITOR_H */
```

#### Fix `anarchy_device.h`

Update `anarchy_device.h` to resolve syntax errors:

```c
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
    struct device *dev;
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
    
    /* Ring buffers */
    struct anarchy_ring tx_ring;
    struct anarchy_ring rx_ring;
    
    /* Performance monitoring */
    struct perf_monitor perf_monitor;
    
    /* Power management */
    struct power_profile power_profile;
    
    /* Workqueues */
    struct workqueue_struct *wq;
    struct work_struct init_work;
    struct work_struct cleanup_work;
    
    /* Statistics */
    atomic_t ref_count;
    
    /* Performance monitoring (pointer to avoid redefinition) */
    struct perf_monitor *perf;
    
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
```

### 1.2. Update Thunderbolt API Usage

The Linux kernel's Thunderbolt API has evolved. Update your code to use the current API:

```c
/* Example of how to properly use the Thunderbolt API in your code */

#include <linux/thunderbolt.h>

/* Use the kernel's ring_frame structure */
static int anarchy_setup_ring(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    /* Create a Thunderbolt ring */
    ring->ring = tb_ring_alloc_tx(adev->service, 0, ring_callback, adev);
    if (!ring->ring)
        return -ENOMEM;
    
    /* Start the ring */
    return tb_ring_start(ring->ring);
}

/* Ring callback function */
static void ring_callback(struct tb_ring *ring, struct ring_frame *frame, bool canceled)
{
    struct anarchy_device *adev = tb_ring_get_drvdata(ring);
    
    /* Process the frame */
    if (!canceled) {
        /* Handle successful transfer */
    } else {
        /* Handle canceled transfer */
    }
}
```

### 1.3. Update PCIe Error Handling

Modern Linux kernels use the Advanced Error Reporting (AER) subsystem for PCIe error handling:

```c
/* Example of how to use the kernel's AER subsystem */

#include <linux/pci.h>
#include <linux/aer.h>

static pci_ers_result_t anarchy_pcie_error_detected(struct pci_dev *pdev,
                                                  enum pci_channel_state state)
{
    struct anarchy_device *adev = pci_get_drvdata(pdev);
    
    switch (state) {
    case pci_channel_io_normal:
        return PCI_ERS_RESULT_CAN_RECOVER;
    case pci_channel_io_frozen:
        /* Handle frozen state */
        return PCI_ERS_RESULT_NEED_RESET;
    case pci_channel_io_perm_failure:
        /* Handle permanent failure */
        return PCI_ERS_RESULT_DISCONNECT;
    }
    
    return PCI_ERS_RESULT_NEED_RESET;
}

static const struct pci_error_handlers anarchy_pcie_err_handlers = {
    .error_detected = anarchy_pcie_error_detected,
    .mmio_enabled = anarchy_pcie_mmio_enabled,
    .slot_reset = anarchy_pcie_slot_reset,
    .resume = anarchy_pcie_resume,
};

static struct pci_driver anarchy_pci_driver = {
    .name = "anarchy_egpu",
    .id_table = anarchy_pci_ids,
    .probe = anarchy_pci_probe,
    .remove = anarchy_pci_remove,
    .err_handler = &anarchy_pcie_err_handlers,
};
```

## 2. Implementing Core Components

After fixing the compilation errors, you need to implement the core components of the GPU passthrough system.

### 2.1. Thunderbolt Controller Interface

The Thunderbolt Controller Interface provides low-level access to the Thunderbolt controller on your Linux machine.

#### Key Implementation Steps:

1. **Initialize the Thunderbolt Controller**:

```c
static int anarchy_tb_init(struct anarchy_device *adev)
{
    int ret;
    
    /* Initialize Thunderbolt controller */
    ret = tb_nhi_init(adev->pdev);
    if (ret) {
        dev_err(adev->dev, "Failed to initialize Thunderbolt controller: %d\n", ret);
        return ret;
    }
    
    /* Configure controller for host-to-host mode */
    ret = tb_nhi_config_host_to_host(adev->pdev);
    if (ret) {
        dev_err(adev->dev, "Failed to configure host-to-host mode: %d\n", ret);
        goto err_exit;
    }
    
    /* Enable XDomain mode */
    ret = tb_nhi_enable_xdomain(adev->pdev);
    if (ret) {
        dev_err(adev->dev, "Failed to enable XDomain mode: %d\n", ret);
        goto err_exit;
    }
    
    return 0;
    
err_exit:
    tb_nhi_exit(adev->pdev);
    return ret;
}
```

2. **Configure USB4 Connection Parameters**:

```c
static int anarchy_usb4_config(struct anarchy_device *adev)
{
    /* Increase stability timeouts */
    adev->usb4_config.stability_timeout_ms = 10000;  // 10 seconds
    adev->usb4_config.min_stable_time_ms = 8000;     // 8 seconds
    adev->usb4_config.connection_debounce_ms = 2000; // 2 seconds
    
    /* Configure USB4 parameters */
    return tb_usb4_config_set(adev->pdev, &adev->usb4_config);
}
```

### 2.2. XDomain Service Manager

The XDomain Service Manager implements the XDomain discovery protocol and manages service advertisement and discovery.

#### Key Implementation Steps:

1. **Define GPU Service**:

```c
static const struct tb_service_id anarchy_gpu_service_table[] = {
    {
        .match_flags = TBSVC_MATCH_PROTOCOL_KEY | 
                      TBSVC_MATCH_PROTOCOL_ID |
                      TBSVC_MATCH_PROTOCOL_VERSION,
        .protocol_key = TB_SERVICE_KEY_GPU,  // Define a GPU-specific key
        .protocol_id = 0x1,
        .protocol_version = 0x1,
        .keys = {
            /* Add Windows-specific keys for better compatibility */
            { .key = "DeviceType", .value = "GPU" },
            { .key = "VendorID", .value = "0x10DE" },
            { .key = "DeviceID", .value = "0x2684" },
            { .key = "DriverCompatibility", .value = "WDDM3.0" },
            { NULL, NULL }
        }
    },
    { }
};
```

2. **Register XDomain Service**:

```c
static int anarchy_xdomain_register(struct anarchy_device *adev)
{
    int ret;
    
    /* Create XDomain service */
    adev->service = tb_service_create(adev->pdev, "NVIDIA RTX 4090 External GPU",
                                     anarchy_gpu_service_table);
    if (IS_ERR(adev->service)) {
        ret = PTR_ERR(adev->service);
        dev_err(adev->dev, "Failed to create XDomain service: %d\n", ret);
        return ret;
    }
    
    /* Register service */
    ret = tb_service_register(adev->service);
    if (ret) {
        dev_err(adev->dev, "Failed to register XDomain service: %d\n", ret);
        tb_service_destroy(adev->service);
        adev->service = NULL;
        return ret;
    }
    
    return 0;
}
```

### 2.3. PCIe Device Emulator

The PCIe Device Emulator creates a virtual PCIe device that appears as an RTX 4090 to the Windows machine.

#### Key Implementation Steps:

1. **Set Up PCIe Configuration Space**:

```c
static void anarchy_pcie_setup_config_space(struct anarchy_device *adev)
{
    /* Set vendor and device IDs */
    anarchy_pcie_write_config_word(adev, 0x00, 0x10DE); // NVIDIA
    anarchy_pcie_write_config_word(adev, 0x02, 0x2684); // RTX 4090
    
    /* Set class code (VGA compatible controller) */
    anarchy_pcie_write_config_byte(adev, 0x0B, 0x03);  // Base class (display controller)
    anarchy_pcie_write_config_byte(adev, 0x0A, 0x00);  // Sub class (VGA compatible)
    anarchy_pcie_write_config_byte(adev, 0x09, 0x00);  // Programming interface
    
    /* Set subsystem vendor and device IDs */
    anarchy_pcie_write_config_word(adev, 0x2C, 0x10DE); // NVIDIA
    anarchy_pcie_write_config_word(adev, 0x2E, 0x2684); // RTX 4090
    
    /* Configure BARs (Base Address Registers) */
    anarchy_pcie_setup_bars(adev);
}
```

2. **Implement PCIe Link Training**:

```c
int anarchy_pcie_train_link(struct anarchy_device *adev)
{
    int ret;
    int retry_count = 0;
    const int max_retries = 5;
    
    /* Reset link before training */
    anarchy_pcie_reset_link(adev);
    
    while (retry_count < max_retries) {
        /* Start link training sequence */
        ret = anarchy_pcie_start_link_training(adev);
        if (ret) {
            dev_err(adev->dev, "Link training failed, attempt %d: %d\n", 
                   retry_count, ret);
            retry_count++;
            msleep(100 * retry_count);  // Increasing delay between retries
            continue;
        }
        
        /* Wait for link to reach L0 state */
        ret = anarchy_pcie_wait_for_link_up(adev, 1000);
        if (ret) {
            dev_err(adev->dev, "Link failed to reach L0, attempt %d: %d\n", 
                   retry_count, ret);
            retry_count++;
            msleep(100 * retry_count);
            continue;
        }
        
        /* Verify link speed and width */
        if (!anarchy_pcie_verify_link_capabilities(adev)) {
            dev_err(adev->dev, "Link capabilities below requirements\n");
            retry_count++;
            continue;
        }
        
        /* Link is up and verified */
        dev_info(adev->dev, "PCIe link trained successfully\n");
        return 0;
    }
    
    dev_err(adev->dev, "Failed to train PCIe link after %d attempts\n", max_retries);
    return -ETIMEDOUT;
}
```

### 2.4. GPU Command Processor

The GPU Command Processor executes GPU commands from Windows on the Linux GPU.

#### Key Implementation Steps:

1. **Initialize Command Processor**:

```c
static int anarchy_cmd_init(struct anarchy_device *adev)
{
    int ret;
    
    /* Allocate command processor */
    adev->cmd_processor = kzalloc(sizeof(struct anarchy_cmd_processor), GFP_KERNEL);
    if (!adev->cmd_processor)
        return -ENOMEM;
    
    /* Initialize command queue */
    INIT_LIST_HEAD(&adev->cmd_processor->cmd_queue);
    spin_lock_init(&adev->cmd_processor->queue_lock);
    
    /* Create command processing thread */
    ret = anarchy_cmd_create_thread(adev);
    if (ret) {
        kfree(adev->cmd_processor);
        adev->cmd_processor = NULL;
        return ret;
    }
    
    return 0;
}
```

2. **Process GPU Commands**:

```c
static int anarchy_cmd_process(struct anarchy_device *adev, struct anarchy_cmd *cmd)
{
    int ret = 0;
    
    switch (cmd->type) {
    case ANARCHY_CMD_INIT_DEVICE:
        ret = anarchy_cmd_init_device(adev, cmd);
        break;
    case ANARCHY_CMD_CREATE_CONTEXT:
        ret = anarchy_cmd_create_context(adev, cmd);
        break;
    case ANARCHY_CMD_ALLOC_MEMORY:
        ret = anarchy_cmd_alloc_memory(adev, cmd);
        break;
    case ANARCHY_CMD_SUBMIT_COMMAND_BUFFER:
        ret = anarchy_cmd_submit_command_buffer(adev, cmd);
        break;
    case ANARCHY_CMD_PRESENT:
        ret = anarchy_cmd_present(adev, cmd);
        break;
    default:
        dev_err(adev->dev, "Unknown command type: %d\n", cmd->type);
        ret = -EINVAL;
        break;
    }
    
    return ret;
}
```

## 3. Testing and Debugging

After implementing the core components, you need to test and debug the system.

### 3.1. Testing Thunderbolt Connection

Test the Thunderbolt connection between your Linux and Windows machines:

```bash
# On Linux machine
sudo modprobe anarchy_egpu
sudo dmesg | grep anarchy
```

Check for successful initialization and connection messages.

### 3.2. Debugging PCIe Device Emulation

Debug the PCIe device emulation:

```bash
# On Linux machine
sudo cat /sys/kernel/debug/anarchy_egpu/pcie_state
```

This should show the current state of the PCIe link, including link speed, width, and training status.

### 3.3. Testing GPU Command Processing

Test the GPU command processing:

```bash
# On Linux machine
sudo cat /sys/kernel/debug/anarchy_egpu/cmd_stats
```

This should show statistics about command processing, including number of commands processed, errors, and performance metrics.

## 4. Performance Optimization

After the basic functionality is working, you can optimize performance.

### 4.1. Bandwidth Optimization

Implement bandwidth optimization techniques:

```c
static int anarchy_bandwidth_init(struct anarchy_device *adev)
{
    /* Initialize bandwidth monitoring */
    adev->bandwidth_monitor = kzalloc(sizeof(struct anarchy_bandwidth_monitor), GFP_KERNEL);
    if (!adev->bandwidth_monitor)
        return -ENOMEM;
    
    /* Set initial bandwidth parameters */
    adev->bandwidth_monitor->target_bandwidth = 32 * 1024 * 1024 * 1024ULL; // 32 GB/s
    adev->bandwidth_monitor->available_bandwidth = 5 * 1024 * 1024 * 1024ULL; // 5 GB/s (Thunderbolt 4)
    adev->bandwidth_monitor->compression_ratio = 4; // Target 4:1 compression
    
    /* Initialize compression */
    return anarchy_compression_init(adev);
}
```

### 4.2. Latency Optimization

Implement latency optimization techniques:

```c
static int anarchy_latency_init(struct anarchy_device *adev)
{
    /* Initialize latency monitoring */
    adev->latency_monitor = kzalloc(sizeof(struct anarchy_latency_monitor), GFP_KERNEL);
    if (!adev->latency_monitor)
        return -ENOMEM;
    
    /* Set latency targets */
    adev->latency_monitor->target_latency_ns = 5 * 1000 * 1000; // 5 ms
    adev->latency_monitor->command_latency_ns = 1 * 1000 * 1000; // 1 ms
    adev->latency_monitor->transfer_latency_ns = 2 * 1000 * 1000; // 2 ms
    
    /* Initialize latency optimization */
    return anarchy_latency_optimization_init(adev);
}
```

### 4.3. Adaptive Quality Control

Implement adaptive quality control:

```c
static int anarchy_quality_control_init(struct anarchy_device *adev)
{
    /* Initialize quality control */
    adev->quality_control = kzalloc(sizeof(struct anarchy_quality_control), GFP_KERNEL);
    if (!adev->quality_control)
        return -ENOMEM;
    
    /* Set initial quality parameters */
    adev->quality_control->resolution_scale = 100; // 100%
    adev->quality_control->texture_quality = 100; // 100%
    adev->quality_control->target_fps = 60;
    
    /* Initialize quality control system */
    return anarchy_quality_control_system_init(adev);
}
```

## 5. Troubleshooting

### 5.1. Connection Issues

If you experience connection issues:

1. **Check USB4/Thunderbolt Cable**:
   - Ensure you're using a high-quality USB4/Thunderbolt cable
   - Try a different cable if available

2. **Check Thunderbolt Controller Status**:
   ```bash
   # On Linux machine
   sudo boltctl list
   ```

3. **Increase Connection Stability Timeouts**:
   ```c
   /* In anarchy_usb4_config function */
   adev->usb4_config.stability_timeout_ms = 15000;  // Increase to 15 seconds
   adev->usb4_config.min_stable_time_ms = 10000;    // Increase to 10 seconds
   ```

### 5.2. Device Recognition Issues

If the Windows machine doesn't recognize the device:

1. **Check PCIe Configuration Space**:
   ```bash
   # On Linux machine
   sudo cat /sys/kernel/debug/anarchy_egpu/pcie_config_space
   ```

2. **Verify Device IDs**:
   - Ensure the vendor ID (0x10DE) and device ID (0x2684) are correctly set
   - Check that the class code is set to 0x030000 (VGA compatible controller)

3. **Check XDomain Service**:
   ```bash
   # On Linux machine
   sudo cat /sys/kernel/debug/anarchy_egpu/xdomain_service
   ```

### 5.3. Performance Issues

If you experience performance issues:

1. **Check Bandwidth Utilization**:
   ```bash
   # On Linux machine
   sudo cat /sys/kernel/debug/anarchy_egpu/bandwidth_stats
   ```

2. **Check Latency Metrics**:
   ```bash
   # On Linux machine
   sudo cat /sys/kernel/debug/anarchy_egpu/latency_stats
   ```

3. **Adjust Quality Control Parameters**:
   ```bash
   # On Linux machine
   echo "resolution_scale 75" > /sys/kernel/debug/anarchy_egpu/quality_control
   echo "texture_quality 75" > /sys/kernel/debug/anarchy_egpu/quality_control
   ```

## Next Steps

After implementing and testing the basic functionality, you can proceed to more advanced features:

1. **DirectX Support**: Implement DirectX support through translation layers
2. **Game Compatibility**: Test with various games and implement game-specific optimizations
3. **User Interface**: Create a user-friendly interface for configuration and monitoring
4. **Documentation**: Create comprehensive documentation for users and developers

By following this implementation guide, you should be able to create a functional GPU passthrough system that allows your Linux machine's RTX 4090 to be used for gaming on your Windows Legion Go over USB4/Thunderbolt.