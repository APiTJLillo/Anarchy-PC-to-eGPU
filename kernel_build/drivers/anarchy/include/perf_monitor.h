#ifndef ANARCHY_PERF_MONITOR_H
#define ANARCHY_PERF_MONITOR_H

#include <linux/types.h>
#include "forward.h"

/* Performance state structure */
struct perf_state {
    u32 gpu_clock;      /* Current GPU clock in MHz */
    u32 mem_clock;      /* Current memory clock in MHz */
    u32 power_draw;     /* Current power draw in watts */
    u32 temperature;    /* GPU temperature in Celsius */
    u32 fan_speed;      /* Fan speed percentage */
    u32 gpu_util;       /* GPU utilization percentage */
    u32 mem_util;       /* Memory utilization percentage */
    u32 vram_used;      /* VRAM usage in megabytes */
    u32 pcie_util;      /* PCIe bandwidth utilization percentage */
};

/* Performance monitor structure */
struct perf_monitor {
    struct anarchy_device *adev;
    bool enabled;
    unsigned int update_interval;
    struct delayed_work update_work;
    spinlock_t lock;
    struct perf_state current_state;
};

/* Performance monitoring interface */
int anarchy_perf_init(struct anarchy_device *adev);
int anarchy_perf_start(struct anarchy_device *adev);
void anarchy_perf_stop(struct anarchy_device *adev);
void anarchy_perf_exit(struct anarchy_device *adev);
int anarchy_perf_get_state(struct anarchy_device *adev, struct perf_state *state);

/* Internal initialization functions */
int init_performance_monitoring(struct anarchy_device *adev);
void cleanup_performance_monitoring(struct anarchy_device *adev);

#endif /* ANARCHY_PERF_MONITOR_H */
