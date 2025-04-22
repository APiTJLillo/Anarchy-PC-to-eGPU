#ifndef ANARCHY_PERF_MONITOR_H
#define ANARCHY_PERF_MONITOR_H

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include "forward.h"
#include "anarchy_device.h"

/* Performance monitoring thresholds */
#define PERF_UPDATE_INTERVAL_MS   1000    /* 1 second update interval */
#define TEMP_WARNING_THRESHOLD    80      /* 80°C warning threshold */
#define TEMP_CRITICAL_THRESHOLD   87      /* 87°C critical threshold */
#define POWER_WARNING_THRESHOLD   200     /* 200W warning threshold */
#define UTILIZATION_THRESHOLD     90      /* 90% GPU utilization */
#define MIN_POWER_LIMIT          150     /* Minimum power limit in watts */

/* Performance monitoring state */
struct perf_stats {
    u32 temperature;
    u32 power_draw;
    u32 gpu_util;
    u32 vram_util;
    u32 pcie_bandwidth;
};

/* Performance monitor structure */
struct perf_monitor {
    struct workqueue_struct *wq;
    struct delayed_work work;
    struct perf_stats stats;
    bool throttling;
    spinlock_t lock;
};

/* Core functions */
int init_performance_monitoring(struct anarchy_device *adev);
void cleanup_performance_monitoring(struct anarchy_device *adev);
int anarchy_perf_update(struct anarchy_device *adev);
void anarchy_perf_get_stats(struct anarchy_device *adev, struct perf_state *state);

#endif /* ANARCHY_PERF_MONITOR_H */
