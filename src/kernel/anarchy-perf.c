#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "include/perf_monitor.h"
#include "include/anarchy_device.h"

static void anarchy_perf_update_work(struct work_struct *work)
{
    struct perf_monitor *monitor = container_of(to_delayed_work(work),
                                              struct perf_monitor,
                                              update_work);
    struct anarchy_device *adev = monitor->adev;
    struct perf_state state;
    
    if (!monitor->enabled)
        return;
        
    /* Read current performance metrics */
    state.gpu_clock = anarchy_gpu_get_clock(adev);
    state.mem_clock = anarchy_gpu_get_mem_clock(adev);
    state.power_draw = anarchy_gpu_get_power(adev);
    state.temperature = anarchy_gpu_get_temp(adev);
    state.fan_speed = anarchy_gpu_get_fan(adev);
    state.gpu_util = anarchy_gpu_get_util(adev);
    state.mem_util = anarchy_gpu_get_mem_util(adev);
    state.pcie_util = anarchy_pcie_get_util(adev);
    state.vram_used = anarchy_gpu_get_vram_used(adev);
    
    spin_lock(&monitor->lock);
    monitor->current_state = state;
    spin_unlock(&monitor->lock);
    
    /* Schedule next update */
    if (monitor->enabled) {
        schedule_delayed_work(&monitor->update_work,
                            msecs_to_jiffies(monitor->update_interval));
    }
}

int anarchy_perf_init(struct anarchy_device *adev)
{
    struct perf_monitor *monitor;
    
    if (!adev)
        return -EINVAL;
        
    monitor = &adev->perf_monitor;
    monitor->adev = adev;
    monitor->enabled = false;
    monitor->update_interval = 1000; /* Default 1 second */
    
    /* Initialize work queue and lock */
    monitor->wq = create_singlethread_workqueue("anarchy-perf");
    if (!monitor->wq)
        return -ENOMEM;
        
    INIT_DELAYED_WORK(&monitor->update_work, anarchy_perf_update_work);
    spin_lock_init(&monitor->lock);
    
    return 0;
}

void anarchy_perf_exit(struct anarchy_device *adev)
{
    struct perf_monitor *monitor;
    
    if (!adev)
        return;
        
    monitor = &adev->perf_monitor;
    anarchy_perf_stop(adev);
    
    if (monitor->wq) {
        destroy_workqueue(monitor->wq);
        monitor->wq = NULL;
    }
}

int anarchy_perf_start(struct anarchy_device *adev)
{
    struct perf_monitor *monitor;
    
    if (!adev)
        return -EINVAL;
        
    monitor = &adev->perf_monitor;
    monitor->enabled = true;
    
    /* Schedule first update */
    schedule_delayed_work(&monitor->update_work,
                         msecs_to_jiffies(monitor->update_interval));
                         
    return 0;
}

void anarchy_perf_stop(struct anarchy_device *adev)
{
    struct perf_monitor *monitor;
    
    if (!adev)
        return;
        
    monitor = &adev->perf_monitor;
    monitor->enabled = false;
    cancel_delayed_work_sync(&monitor->update_work);
}

int anarchy_perf_get_state(struct anarchy_device *adev, struct perf_state *state)
{
    struct perf_monitor *monitor;
    
    if (!adev || !state)
        return -EINVAL;
        
    monitor = &adev->perf_monitor;
    
    spin_lock(&monitor->lock);
    *state = monitor->current_state;
    spin_unlock(&monitor->lock);
    
    return 0;
}

EXPORT_SYMBOL_GPL(anarchy_perf_init);
EXPORT_SYMBOL_GPL(anarchy_perf_exit);
EXPORT_SYMBOL_GPL(anarchy_perf_start);
EXPORT_SYMBOL_GPL(anarchy_perf_stop);
EXPORT_SYMBOL_GPL(anarchy_perf_get_state);
