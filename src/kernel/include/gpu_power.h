#ifndef ANARCHY_GPU_POWER_H
#define ANARCHY_GPU_POWER_H

#include <linux/types.h>
#include "forward.h"

/* Power limits */
#define GPU_POWER_LIMIT_MIN    150
#define GPU_POWER_LIMIT_MAX    250
#define GPU_POWER_LIMIT_DEFAULT 175

/* Power management functions */
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, u32 limit);
int anarchy_gpu_get_power_limit(struct anarchy_device *adev, u32 *limit);
int anarchy_gpu_power_down(struct anarchy_device *adev);
int anarchy_gpu_power_up(struct anarchy_device *adev);

#endif /* ANARCHY_GPU_POWER_H */
