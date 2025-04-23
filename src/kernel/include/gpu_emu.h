#ifndef ANARCHY_GPU_EMU_H
#define ANARCHY_GPU_EMU_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include "anarchy_device.h"

/* GPU Emulation States */
#define GPU_EMU_STATE_DISABLED  0
#define GPU_EMU_STATE_INIT      1
#define GPU_EMU_STATE_RUNNING   2

/* GPU Performance State */
struct gpu_emu_perf_state {
    u32 gpu_clock;
    u32 mem_clock;
    u32 power_draw;
    u32 temperature;
    u32 fan_speed;
    u32 gpu_util;
    u32 mem_util;
    u32 vram_used;
    u32 pcie_rx_bw;
    u32 pcie_tx_bw;
};

/* GPU Emulation Config */
struct gpu_emu_config {
    bool enabled;
    bool cuda_support;
    bool rtx_support;
    bool dlss_support;
    unsigned int core_count;
    unsigned int ray_cores;
    unsigned int tensor_cores;
    unsigned int sm_version;
    unsigned int vram_size;
    unsigned int base_clock;
    unsigned int boost_clock;
    unsigned int mem_clock;
};

/* GPU Emulation Interface */
struct gpu_emu_interface {
    struct anarchy_device *adev;
    spinlock_t lock;
    int state;
    struct gpu_emu_config config;
    struct gpu_emu_perf_state perf;
    void *fb_base;
    size_t fb_size;
};

/* GPU Performance Registers */
#define GPU_EMU_CLOCK_OFFSET     0x1000
#define GPU_EMU_MEM_OFFSET      0x1004
#define GPU_EMU_POWER_OFFSET    0x1008
#define GPU_EMU_TEMP_OFFSET     0x100C
#define GPU_EMU_FAN_OFFSET      0x1010
#define GPU_UTIL_OFFSET     0x1014
#define MEM_UTIL_OFFSET     0x1018
#define VRAM_USED_OFFSET    0x101C
#define GPU_PCIE_RX_COUNTER     0x1020
#define GPU_PCIE_TX_COUNTER     0x1024
#define PWR_LIMIT_OFFSET    0x1028
#define PWR_LIMIT_UPDATE    0x102C
#define FAN_CONTROL_UPDATE  0x1030

/* GPU State Functions */
u32 anarchy_gpu_get_clock(struct anarchy_device *adev);
u32 anarchy_gpu_get_mem_clock(struct anarchy_device *adev);
u32 anarchy_gpu_get_power(struct anarchy_device *adev);
u32 anarchy_gpu_get_temp(struct anarchy_device *adev);
u32 anarchy_gpu_get_fan(struct anarchy_device *adev);
u32 anarchy_gpu_get_util(struct anarchy_device *adev);
u32 anarchy_gpu_get_mem_util(struct anarchy_device *adev);
u32 anarchy_gpu_get_vram_used(struct anarchy_device *adev);

/* GPU Control Functions */
int anarchy_gpu_set_fan_speed(struct anarchy_device *adev, u32 speed);
int anarchy_gpu_emu_set_power_limit(struct anarchy_device *adev, u32 limit);
int anarchy_gpu_set_clocks(struct anarchy_device *adev, u32 gpu_clock, u32 mem_clock);

/* GPU Emulation Core Functions */
int anarchy_gpu_emu_init(struct anarchy_device *adev);
void anarchy_gpu_emu_exit(struct anarchy_device *adev);
int anarchy_gpu_emu_start(struct anarchy_device *adev);
int anarchy_gpu_emu_stop(struct anarchy_device *adev);
int anarchy_gpu_emu_handle_mmio(struct gpu_emu_interface *emu, u32 offset,
                               u32 *value, bool is_write);
int anarchy_gpu_emu_map_memory(struct anarchy_device *adev, u64 addr,
                              size_t size);

#endif /* ANARCHY_GPU_EMU_H */
