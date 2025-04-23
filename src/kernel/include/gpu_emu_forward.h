#ifndef ANARCHY_GPU_EMU_FORWARD_H
#define ANARCHY_GPU_EMU_FORWARD_H

struct gpu_emu_interface;
struct anarchy_device;

/* GPU Emulation States */
enum gpu_emu_state {
    GPU_EMU_STATE_DISABLED = 0,
    GPU_EMU_STATE_INIT,
    GPU_EMU_STATE_RUNNING,
    GPU_EMU_STATE_ERROR
};

#endif /* ANARCHY_GPU_EMU_FORWARD_H */ 