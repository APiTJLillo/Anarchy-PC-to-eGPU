#ifndef ANARCHY_GPU_EMU_H
#define ANARCHY_GPU_EMU_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include "anarchy_device.h"
#include "gpu_emu_forward.h"

/* GPU Performance Registers */
#define GPU_EMU_CLOCK_OFFSET     0x1000
#define GPU_EMU_MEM_OFFSET      0x1004
#define GPU_EMU_POWER_OFFSET    0x1008
#define GPU_EMU_TEMP_OFFSET     0x100C
#define GPU_EMU_FAN_OFFSET      0x1010
#define GPU_UTIL_OFFSET     0x1014
#define MEM_UTIL_OFFSET     0x1018
#define VRAM_USED_OFFSET    0x101C
#define PCIE_RX_COUNTER     0x1020
#define PCIE_TX_COUNTER     0x1024
#define PWR_LIMIT_OFFSET    0x1028
#define PWR_LIMIT_UPDATE    0x102C
#define FAN_CONTROL_UPDATE  0x1030

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

/* Performance State */
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

/* Emulation Interface */
struct gpu_emu_interface {
    struct anarchy_device *adev;
    enum gpu_emu_state state;
    struct gpu_emu_config config;
    struct gpu_emu_perf_state perf;
    void *mmio_base;
    void *fb_base;
    unsigned long mmio_size;
    unsigned long fb_size;
    spinlock_t lock;
};

/* GPU Access Functions */
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
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, u32 limit);
int anarchy_gpu_set_clocks(struct anarchy_device *adev, u32 gpu_clock, u32 mem_clock);

/* Emulation Functions */
int anarchy_gpu_emu_init(struct anarchy_device *adev);
void anarchy_gpu_emu_cleanup(struct anarchy_device *adev);
int anarchy_gpu_emu_start(struct anarchy_device *adev);
void anarchy_gpu_emu_stop(struct anarchy_device *adev);
int anarchy_gpu_emu_handle_mmio(struct anarchy_device *adev, u32 offset, u32 *value, bool is_write);
int anarchy_gpu_emu_map_memory(struct anarchy_device *adev, u64 addr, u64 size);

#endif /* ANARCHY_GPU_EMU_H */
