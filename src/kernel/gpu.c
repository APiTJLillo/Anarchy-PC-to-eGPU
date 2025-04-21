#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/pci.h>

#include "anarchy-egpu.h"
#include "anarchy-gpu.h"
#include "anarchy-debug.h"
#include "anarchy-perf.h"

/* Maximum number of reset attempts before giving up */
#define ANARCHY_GPU_MAX_RESETS 3

/* Delay between reset attempts in milliseconds */
#define ANARCHY_GPU_RESET_DELAY_MS 1000

/* Timeout for reset operations in milliseconds */
#define ANARCHY_GPU_RESET_TIMEOUT_MS 5000

/* GPU error types */
enum anarchy_gpu_error {
    GPU_ERR_NONE = 0,
    GPU_ERR_TIMEOUT,
    GPU_ERR_ECC,
    GPU_ERR_THERMAL,
    GPU_ERR_POWER,
    GPU_ERR_MEMORY,
    GPU_ERR_ENGINE,
    GPU_ERR_FATAL
};

/* GPU error state tracking */
struct anarchy_gpu_error_state {
    enum anarchy_gpu_error last_error;
    unsigned int reset_count;
    atomic_t stats[GPU_ERR_FATAL + 1];
    struct work_struct recovery_work;
    struct workqueue_struct *recovery_wq;
    spinlock_t lock;
};

/**
 * anarchy_gpu_handle_error - Handle GPU errors
 * @adev: Anarchy device structure
 * @error: Type of error that occurred
 *
 * This function handles various GPU errors and initiates reset if necessary.
 */
void anarchy_gpu_handle_error(struct anarchy_device *adev, enum anarchy_gpu_error error)
{
    unsigned long flags;

    if (!adev || error <= GPU_ERR_NONE || error > GPU_ERR_FATAL)
        return;

    atomic_inc(&adev->error_state.stats[error]);

    spin_lock_irqsave(&adev->error_state.lock, flags);
    adev->error_state.last_error = error;
    spin_unlock_irqrestore(&adev->error_state.lock, flags);

    pr_err("anarchy-egpu: GPU error %d detected\n", error);

    /* Schedule recovery work */
    queue_work(adev->error_state.recovery_wq, &adev->error_state.recovery_work);
}

/**
 * anarchy_gpu_reset_work - Work queue function for GPU reset
 * @work: Work structure
 *
 * This function performs the actual GPU reset sequence.
 */
static void anarchy_gpu_reset_work(struct work_struct *work)
{
    struct anarchy_device *adev = container_of(work, struct anarchy_device,
                                             gpu_reset_work);
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&adev->gpu_reset_lock, flags);
    if (adev->gpu_errors.reset_count >= ANARCHY_GPU_MAX_RESETS) {
        dev_err(adev->dev, "Maximum reset attempts reached\n");
        adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_FAILED;
        spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);
        return;
    }
    spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);

    /* Perform reset sequence */
    ret = anarchy_gpu_reset(adev);
    if (ret) {
        dev_err(adev->dev, "GPU reset failed: %d\n", ret);
        
        spin_lock_irqsave(&adev->gpu_reset_lock, flags);
        adev->gpu_errors.reset_count++;
        adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_FAILED;
        spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);

        /* Schedule another reset attempt after delay */
        msleep(ANARCHY_GPU_RESET_DELAY_MS);
        queue_work(adev->gpu_reset_wq, &adev->gpu_reset_work);
    } else {
        spin_lock_irqsave(&adev->gpu_reset_lock, flags);
        adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_NONE;
        adev->gpu_errors.last_reset_time = ktime_get();
        atomic_set(&adev->gpu_errors.pending_faults, 0);
        spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);

        dev_info(adev->dev, "GPU reset completed successfully\n");
    }
}

/**
 * anarchy_gpu_reset - Perform GPU reset sequence
 * @adev: Anarchy device structure
 *
 * This function implements the actual GPU reset sequence, including:
 * 1. PCIe link reset
 * 2. GPU power state management
 * 3. Memory controller reset
 * 4. Engine reset
 * 5. State recovery
 */
int anarchy_gpu_reset(struct anarchy_device *adev)
{
    unsigned long flags, timeout;
    int ret = 0;
    u32 status;

    if (!adev)
        return -EINVAL;

    spin_lock_irqsave(&adev->gpu_reset_lock, flags);
    adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_IN_PROGRESS;
    spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);

    dev_info(adev->dev, "Starting GPU reset sequence\n");

    /* Stop all DMA transfers */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);

    /* Step 1: PCIe link reset */
    dev_dbg(adev->dev, "Resetting PCIe link\n");
    ret = anarchy_pcie_check_link(adev);
    if (ret) {
        dev_err(adev->dev, "PCIe link reset failed: %d\n", ret);
        goto fail;
    }

    /* Wait for PCIe link to stabilize */
    msleep(GPU_LINK_TIMEOUT);

    /* Step 2: Power state management */
    dev_dbg(adev->dev, "Managing GPU power state\n");
    
    /* Mask all interrupts during reset */
    ret = anarchy_pcie_write_config(adev, GPU_INTR_MASK_REG, 4, 0xFFFFFFFF);
    if (ret) {
        dev_err(adev->dev, "Failed to mask interrupts: %d\n", ret);
        goto fail;
    }

    /* Force GPU to D3 state */
    ret = anarchy_pcie_write_config(adev, PCI_POWER_CONTROL, 4, GPU_POWER_D3);
    if (ret) {
        dev_err(adev->dev, "Failed to set D3 state: %d\n", ret);
        goto fail;
    }

    msleep(100);

    /* Bring GPU back to D0 state */
    ret = anarchy_pcie_write_config(adev, PCI_POWER_CONTROL, 4, GPU_POWER_D0);
    if (ret) {
        dev_err(adev->dev, "Failed to set D0 state: %d\n", ret);
        goto fail;
    }

    /* Wait for power state transition */
    timeout = jiffies + msecs_to_jiffies(GPU_INIT_TIMEOUT);
    while (time_before(jiffies, timeout)) {
        ret = anarchy_pcie_read_config(adev, PCI_POWER_STATUS, 4, &status);
        if (ret) {
            dev_err(adev->dev, "Failed to read power status: %d\n", ret);
            goto fail;
        }
        
        if ((status & GPU_POWER_STATE_MASK) == GPU_POWER_D0)
            break;
            
        msleep(10);
    }

    if ((status & GPU_POWER_STATE_MASK) != GPU_POWER_D0) {
        dev_err(adev->dev, "GPU failed to enter D0 state\n");
        ret = -ETIMEDOUT;
        goto fail;
    }

    /* Step 3: Memory controller reset */
    dev_dbg(adev->dev, "Resetting memory controller\n");
    ret = anarchy_pcie_write_config(adev, GPU_MEM_CTRL_RESET, 4, GPU_MEM_RESET_BIT);
    if (ret) {
        dev_err(adev->dev, "Failed to assert memory reset: %d\n", ret);
        goto fail;
    }

    msleep(100);

    ret = anarchy_pcie_write_config(adev, GPU_MEM_CTRL_RESET, 4, 0);
    if (ret) {
        dev_err(adev->dev, "Failed to deassert memory reset: %d\n", ret);
        goto fail;
    }

    /* Step 4: Engine reset */
    dev_dbg(adev->dev, "Resetting GPU engines\n");
    ret = anarchy_pcie_write_config(adev, GPU_ENGINE_CTRL_RESET, 4, GPU_ENGINE_RESET_BIT);
    if (ret) {
        dev_err(adev->dev, "Failed to assert engine reset: %d\n", ret);
        goto fail;
    }

    msleep(100);

    ret = anarchy_pcie_write_config(adev, GPU_ENGINE_CTRL_RESET, 4, 0);
    if (ret) {
        dev_err(adev->dev, "Failed to deassert engine reset: %d\n", ret);
        goto fail;
    }

    /* Step 5: Restart DMA rings */
    dev_dbg(adev->dev, "Restarting DMA rings\n");
    ret = anarchy_ring_start(adev, &adev->tx_ring, true);
    if (ret) {
        dev_err(adev->dev, "Failed to restart TX ring: %d\n", ret);
        goto fail;
    }

    ret = anarchy_ring_start(adev, &adev->rx_ring, false);
    if (ret) {
        dev_err(adev->dev, "Failed to restart RX ring: %d\n", ret);
        anarchy_ring_stop(adev, &adev->tx_ring);
        goto fail;
    }

    /* Step 6: Wait for GPU to become responsive */
    dev_dbg(adev->dev, "Waiting for GPU to initialize\n");
    timeout = jiffies + msecs_to_jiffies(GPU_INIT_TIMEOUT);
    while (time_before(jiffies, timeout)) {
        ret = anarchy_pcie_read_config(adev, GPU_STATUS_REG, 4, &status);
        if (ret) {
            dev_err(adev->dev, "Failed to read GPU status: %d\n", ret);
            goto fail_rings;
        }
            
        if ((status & GPU_ENGINE_STATE_MASK) == GPU_ENGINE_IDLE)
            break;
            
        msleep(10);
    }

    if ((status & GPU_ENGINE_STATE_MASK) != GPU_ENGINE_IDLE) {
        dev_err(adev->dev, "GPU failed to enter idle state\n");
        ret = -ETIMEDOUT;
        goto fail_rings;
    }

    /* Step 7: Restore interrupt mask */
    ret = anarchy_pcie_write_config(adev, GPU_INTR_MASK_REG, 4, 0);
    if (ret) {
        dev_err(adev->dev, "Failed to restore interrupt mask: %d\n", ret);
        goto fail_rings;
    }

    spin_lock_irqsave(&adev->gpu_reset_lock, flags);
    adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_RECOVERY;
    adev->gpu_errors.last_reset_time = ktime_get();
    spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);

    dev_info(adev->dev, "GPU reset sequence completed successfully\n");
    return 0;

fail_rings:
    anarchy_ring_stop(adev, &adev->rx_ring);
    anarchy_ring_stop(adev, &adev->tx_ring);
fail:
    spin_lock_irqsave(&adev->gpu_reset_lock, flags);
    adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_FAILED;
    spin_unlock_irqrestore(&adev->gpu_reset_lock, flags);
    dev_err(adev->dev, "GPU reset sequence failed with error %d\n", ret);
    return ret;
}

/**
 * anarchy_gpu_check_status - Check GPU status
 * @adev: Anarchy device structure
 *
 * This function checks various GPU status indicators and handles any issues.
 */
void anarchy_gpu_check_status(struct anarchy_device *adev)
{
    u32 status;
    
    /* Read GPU status registers */
    /* TODO: Implement actual register reads once we have MMIO access */
    
    /* Check for timeout conditions */
    if (status & GPU_TIMEOUT_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_TIMEOUT);
        
    /* Check for ECC errors */
    if (status & GPU_ECC_ERROR_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_ECC);
        
    /* Check thermal status */
    if (status & GPU_THERMAL_ALERT_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_THERMAL);
        
    /* Check power status */
    if (status & GPU_POWER_ALERT_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_POWER);
        
    /* Check for memory errors */
    if (status & GPU_MEMORY_ERROR_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_MEMORY);
        
    /* Check for engine faults */
    if (status & GPU_ENGINE_FAULT_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_ENGINE);
        
    /* Check for fatal errors */
    if (status & GPU_FATAL_ERROR_MASK)
        anarchy_gpu_handle_error(adev, GPU_ERR_FATAL);
}

/**
 * anarchy_gpu_init - Initialize GPU error handling
 * @adev: Anarchy device structure
 *
 * This function initializes the GPU error handling subsystem.
 */
int anarchy_gpu_init(struct anarchy_device *adev)
{
    /* Initialize error handling state */
    adev->gpu_errors.last_error = GPU_ERR_NONE;
    adev->gpu_errors.reset_state = ANARCHY_GPU_RESET_NONE;
    adev->gpu_errors.reset_count = 0;
    adev->gpu_errors.last_reset_time = ktime_get();
    atomic_set(&adev->gpu_errors.pending_faults, 0);

    /* Initialize statistics */
    atomic_set(&adev->gpu_errors.stats.timeout_errors, 0);
    atomic_set(&adev->gpu_errors.stats.ecc_errors, 0);
    atomic_set(&adev->gpu_errors.stats.thermal_errors, 0);
    atomic_set(&adev->gpu_errors.stats.power_errors, 0);
    atomic_set(&adev->gpu_errors.stats.memory_errors, 0);
    atomic_set(&adev->gpu_errors.stats.engine_errors, 0);
    atomic_set(&adev->gpu_errors.stats.fatal_errors, 0);

    /* Initialize synchronization primitives */
    spin_lock_init(&adev->gpu_reset_lock);
    init_waitqueue_head(&adev->gpu_reset_wait);

    /* Create reset workqueue */
    adev->gpu_reset_wq = create_singlethread_workqueue("anarchy_gpu_reset");
    if (!adev->gpu_reset_wq)
        return -ENOMEM;

    INIT_WORK(&adev->gpu_reset_work, anarchy_gpu_reset_work);

    return 0;
}

/**
 * anarchy_gpu_exit - Clean up GPU error handling
 * @adev: Anarchy device structure
 *
 * This function cleans up the GPU error handling subsystem.
 */
void anarchy_gpu_exit(struct anarchy_device *adev)
{
    if (adev->gpu_reset_wq) {
        destroy_workqueue(adev->gpu_reset_wq);
        adev->gpu_reset_wq = NULL;
    }
}

static void anarchy_gpu_recovery_work_fn(struct work_struct *work)
{
    struct anarchy_device *adev = container_of(work, struct anarchy_device,
                                             error_state.recovery_work);
    enum anarchy_gpu_error last_error;
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&adev->error_state.lock, flags);
    last_error = adev->error_state.last_error;
    spin_unlock_irqrestore(&adev->error_state.lock, flags);

    if (last_error == GPU_ERR_NONE)
        return;

    /* For fatal errors, attempt reset up to 3 times */
    if (last_error == GPU_ERR_FATAL && adev->error_state.reset_count >= 3) {
        pr_err("anarchy-egpu: Fatal error recovery failed after 3 attempts\n");
        return;
    }

    pr_info("anarchy-egpu: Attempting GPU recovery for error %d\n", last_error);

    ret = anarchy_gpu_reset(adev);
    if (ret) {
        pr_err("anarchy-egpu: GPU reset failed with error %d\n", ret);
        adev->error_state.reset_count++;
        /* Schedule another recovery attempt after delay */
        if (last_error == GPU_ERR_FATAL && adev->error_state.reset_count < 3) {
            msleep(1000);
            queue_work(adev->error_state.recovery_wq, &adev->error_state.recovery_work);
        }
    } else {
        spin_lock_irqsave(&adev->error_state.lock, flags);
        adev->error_state.last_error = GPU_ERR_NONE;
        adev->error_state.reset_count = 0;
        spin_unlock_irqrestore(&adev->error_state.lock, flags);
        pr_info("anarchy-egpu: GPU recovery successful\n");
    }
}

int anarchy_gpu_init_error_handling(struct anarchy_device *adev)
{
    int i;

    if (!adev)
        return -EINVAL;

    /* Initialize error state */
    adev->error_state.last_error = GPU_ERR_NONE;
    adev->error_state.reset_count = 0;
    spin_lock_init(&adev->error_state.lock);

    /* Initialize error statistics */
    for (i = 0; i <= GPU_ERR_FATAL; i++)
        atomic_set(&adev->error_state.stats[i], 0);

    /* Create recovery workqueue */
    adev->error_state.recovery_wq = create_singlethread_workqueue("anarchy_gpu_recovery");
    if (!adev->error_state.recovery_wq) {
        pr_err("anarchy-egpu: Failed to create recovery workqueue\n");
        return -ENOMEM;
    }

    INIT_WORK(&adev->error_state.recovery_work, anarchy_gpu_recovery_work_fn);

    return 0;
}

void anarchy_gpu_cleanup_error_handling(struct anarchy_device *adev)
{
    if (!adev)
        return;

    if (adev->error_state.recovery_wq) {
        flush_workqueue(adev->error_state.recovery_wq);
        destroy_workqueue(adev->error_state.recovery_wq);
        adev->error_state.recovery_wq = NULL;
    }
} 