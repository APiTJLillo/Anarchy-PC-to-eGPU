#ifndef ANARCHY_EGPU_H
#define ANARCHY_EGPU_H

#include <linux/types.h>
#include "include/anarchy_device.h"
#include "include/pcie_types.h"
#include "include/common.h"
#include "include/gpu_config.h"
#include "include/thermal.h"

/* Core initialization */
int anarchy_endpoint_init(struct anarchy_device *adev);
int anarchy_pcie_init(struct anarchy_device *adev);
int init_game_compatibility(struct anarchy_device *adev);
int init_performance_monitoring(struct anarchy_device *adev);
int init_command_processor(struct anarchy_device *adev);
void anarchy_pcie_exit(struct anarchy_device *adev);

/* Configuration */
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game);
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, u32 limit);
int anarchy_gpu_set_clocks(struct anarchy_device *adev, u32 core, u32 mem);
int anarchy_gpu_set_memory_config(struct anarchy_device *adev, u32 size);

/* Service configuration */
struct tb_service_config {
    u32 max_lanes;
    u32 max_speed;
    bool endpoint_mode;
};

#endif /* ANARCHY_EGPU_H */
