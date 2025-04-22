#include <linux/module.h>
#include <linux/workqueue.h>
#include "include/gpu_config.h"
#include "include/gpu_emu.h"

/* Performance monitoring thresholds */
#define PERF_UPDATE_INTERVAL_MS   1000    /* 1 second update interval */
#define TEMP_WARNING_THRESHOLD    80      /* 80°C warning threshold */
#define TEMP_CRITICAL_THRESHOLD   87      /* 87°C critical threshold */
#define POWER_WARNING_THRESHOLD   200     /* 200W warning threshold */
#define UTILIZATION_THRESHOLD     90      /* 90% GPU utilization */

struct perf_monitor {
    struct workqueue_struct *wq;
    struct delayed_work work;
    struct {
        u32 temperature;
        u32 power_draw;
        u32 gpu_util;
        u32 vram_util;
        u32 pcie_bandwidth;
    } stats;
    bool throttling;
    spinlock_t lock;
};

static void update_performance(struct work_struct *work)
{
    struct perf_monitor *pm = container_of(to_delayed_work(work),
                                         struct perf_monitor, work);
    struct anarchy_device *adev = container_of(pm, struct anarchy_device,
                                             perf_monitor);
    unsigned long flags;
    
    spin_lock_irqsave(&pm->lock, flags);

    /* Read current stats */
    pm->stats.temperature = readl(adev->mmio_base + TEMP_OFFSET);
    pm->stats.power_draw = readl(adev->mmio_base + POWER_OFFSET);
    pm->stats.gpu_util = readl(adev->mmio_base + UTIL_OFFSET);
    pm->stats.vram_util = readl(adev->mmio_base + VRAM_UTIL_OFFSET);
    pm->stats.pcie_bandwidth = readl(adev->mmio_base + PCIE_BW_OFFSET);

    /* Check for thermal throttling */
    if (pm->stats.temperature >= TEMP_CRITICAL_THRESHOLD) {
        anarchy_gpu_set_power_limit(adev, MIN_POWER_LIMIT);
        pm->throttling = true;
    } else if (pm->stats.temperature <= TEMP_WARNING_THRESHOLD && pm->throttling) {
        /* Recover from throttling */
        anarchy_gpu_set_power_limit(adev, adev->power_limit);
        pm->throttling = false;
    }

    /* Dynamic optimization based on workload */
    if (pm->stats.gpu_util > UTILIZATION_THRESHOLD) {
        /* Heavy gaming workload - optimize for performance */
        optimize_command_processing(adev, NULL);
        anarchy_dma_optimize_transfers(adev);
    }

    spin_unlock_irqrestore(&pm->lock, flags);

    /* Schedule next update */
    queue_delayed_work(pm->wq, &pm->work,
                      msecs_to_jiffies(PERF_UPDATE_INTERVAL_MS));
}

int init_performance_monitoring(struct anarchy_device *adev)
{
    struct perf_monitor *pm;

    pm = kzalloc(sizeof(*pm), GFP_KERNEL);
    if (!pm)
        return -ENOMEM;

    pm->wq = create_singlethread_workqueue("anarchy_perf");
    if (!pm->wq) {
        kfree(pm);
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&pm->work, update_performance);
    spin_lock_init(&pm->lock);
    pm->throttling = false;

    adev->perf_monitor = pm;

    /* Start monitoring */
    queue_delayed_work(pm->wq, &pm->work,
                      msecs_to_jiffies(PERF_UPDATE_INTERVAL_MS));

    return 0;
}
