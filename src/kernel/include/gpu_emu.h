#ifndef ANARCHY_GPU_EMU_H
#define ANARCHY_GPU_EMU_H

#include <linux/types.h>
#include "forward.h"

/* GPU Emulation States */
enum gpu_emu_state {
    GPU_EMU_STATE_DISABLED = 0,
    GPU_EMU_STATE_INIT,
    GPU_EMU_STATE_RUNNING,
    GPU_EMU_STATE_ERROR
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
};

/* Emulation Interface */
struct gpu_emu_interface {
    struct anarchy_device *adev;
    enum gpu_emu_state state;
    struct gpu_emu_config config;
};

/* GPU Emulation Functions */
int anarchy_gpu_emu_init(struct anarchy_device *adev);
void anarchy_gpu_emu_cleanup(struct anarchy_device *adev);
int anarchy_gpu_emu_start(struct anarchy_device *adev);
void anarchy_gpu_emu_stop(struct anarchy_device *adev);
    void *mmio_base;
    void *fb_base;
    unsigned long mmio_size;
    unsigned long fb_size;
};

/* Emulation Functions */
int anarchy_gpu_emu_init(struct anarchy_device *adev);
void anarchy_gpu_emu_cleanup(struct anarchy_device *adev);
int anarchy_gpu_emu_handle_mmio(struct anarchy_device *adev, u32 offset, u32 *value, bool is_write);
int anarchy_gpu_emu_map_memory(struct anarchy_device *adev, u64 addr, u64 size);

#endif /* ANARCHY_GPU_EMU_H */
