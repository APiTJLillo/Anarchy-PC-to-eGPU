#include <linux/module.h>
#include <linux/kernel.h>
#include "include/anarchy_device.h"
#include "include/gpu_emu.h"

/* GPU Access Functions */
u32 anarchy_gpu_get_clock(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.gpu_clock;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_clock);

u32 anarchy_gpu_get_mem_clock(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.mem_clock;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_mem_clock);

u32 anarchy_gpu_get_power(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.power_draw;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_power);

u32 anarchy_gpu_get_temp(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.temperature;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_temp);

u32 anarchy_gpu_get_fan(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.fan_speed;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_fan);

u32 anarchy_gpu_get_util(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.gpu_util;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_util);

u32 anarchy_gpu_get_mem_util(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.mem_util;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_mem_util);

u32 anarchy_gpu_get_vram_used(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return 0;
    return adev->gpu_emu->perf.vram_used;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_get_vram_used);

/* GPU Control Functions */
int anarchy_gpu_set_fan_speed(struct anarchy_device *adev, u32 speed)
{
    if (!adev || !adev->gpu_emu)
        return -EINVAL;
    if (speed > 100)
        return -EINVAL;
    adev->gpu_emu->perf.fan_speed = speed;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_set_fan_speed);

int anarchy_gpu_set_power_limit(struct anarchy_device *adev, u32 limit)
{
    if (!adev || !adev->gpu_emu)
        return -EINVAL;
    /* Assuming max power limit of 450W */
    if (limit > 450)
        return -EINVAL;
    adev->gpu_emu->perf.power_draw = limit;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_set_power_limit);

int anarchy_gpu_set_clocks(struct anarchy_device *adev, u32 gpu_clock, u32 mem_clock)
{
    if (!adev || !adev->gpu_emu)
        return -EINVAL;
    adev->gpu_emu->perf.gpu_clock = gpu_clock;
    adev->gpu_emu->perf.mem_clock = mem_clock;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_set_clocks);

/* Emulation Functions */
int anarchy_gpu_emu_init(struct anarchy_device *adev)
{
    struct gpu_emu_interface *emu;

    if (!adev)
        return -EINVAL;

    emu = kzalloc(sizeof(*emu), GFP_KERNEL);
    if (!emu)
        return -ENOMEM;

    emu->adev = adev;
    emu->state = GPU_EMU_STATE_INIT;
    spin_lock_init(&emu->lock);

    /* Set default configuration */
    emu->config.enabled = true;
    emu->config.cuda_support = true;
    emu->config.core_count = 3584;  /* Default to RTX 3060 Ti */
    emu->config.vram_size = 8192;   /* 8GB VRAM */
    emu->config.base_clock = 1410;
    emu->config.boost_clock = 1665;
    emu->config.mem_clock = 1750;

    adev->gpu_emu = emu;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_emu_init);

void anarchy_gpu_emu_cleanup(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return;

    kfree(adev->gpu_emu);
    adev->gpu_emu = NULL;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_emu_cleanup);

int anarchy_gpu_emu_start(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return -EINVAL;
    adev->gpu_emu->state = GPU_EMU_STATE_RUNNING;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_emu_start);

void anarchy_gpu_emu_stop(struct anarchy_device *adev)
{
    if (!adev || !adev->gpu_emu)
        return;
    adev->gpu_emu->state = GPU_EMU_STATE_DISABLED;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_emu_stop);

int anarchy_gpu_emu_handle_mmio(struct anarchy_device *adev, u32 offset, u32 *value, bool is_write)
{
    struct gpu_emu_interface *emu;
    
    if (!adev || !adev->gpu_emu || !value)
        return -EINVAL;

    emu = adev->gpu_emu;
    
    if (is_write) {
        switch (offset) {
        case GPU_EMU_CLOCK_OFFSET:
            emu->perf.gpu_clock = *value;
            break;
        case GPU_EMU_MEM_OFFSET:
            emu->perf.mem_clock = *value;
            break;
        case GPU_EMU_POWER_OFFSET:
            emu->perf.power_draw = *value;
            break;
        case GPU_EMU_FAN_OFFSET:
            emu->perf.fan_speed = *value;
            break;
        default:
            return -EINVAL;
        }
    } else {
        switch (offset) {
        case GPU_EMU_CLOCK_OFFSET:
            *value = emu->perf.gpu_clock;
            break;
        case GPU_EMU_MEM_OFFSET:
            *value = emu->perf.mem_clock;
            break;
        case GPU_EMU_POWER_OFFSET:
            *value = emu->perf.power_draw;
            break;
        case GPU_EMU_TEMP_OFFSET:
            *value = emu->perf.temperature;
            break;
        case GPU_EMU_FAN_OFFSET:
            *value = emu->perf.fan_speed;
            break;
        case GPU_UTIL_OFFSET:
            *value = emu->perf.gpu_util;
            break;
        case MEM_UTIL_OFFSET:
            *value = emu->perf.mem_util;
            break;
        case VRAM_USED_OFFSET:
            *value = emu->perf.vram_used;
            break;
        default:
            return -EINVAL;
        }
    }
    
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_emu_handle_mmio);

int anarchy_gpu_emu_map_memory(struct anarchy_device *adev, u64 addr, u64 size)
{
    if (!adev || !adev->gpu_emu)
        return -EINVAL;
    
    adev->gpu_emu->fb_base = (void *)addr;
    adev->gpu_emu->fb_size = size;
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_gpu_emu_map_memory);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("GPU Emulation Layer for Anarchy eGPU Driver"); 