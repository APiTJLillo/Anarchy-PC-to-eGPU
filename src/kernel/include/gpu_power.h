#ifndef ANARCHY_GPU_POWER_H
#define ANARCHY_GPU_POWER_H

#include <linux/types.h>
#include "anarchy_device.h"

/* Power limit constants */
#define GPU_POWER_LIMIT_MIN      100  /* 100W minimum */
#define GPU_POWER_LIMIT_MAX      450  /* 450W maximum */
#define GPU_POWER_LIMIT_DEFAULT  300  /* 300W default */

/* GPU Power Management Functions */
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, u32 limit);
int anarchy_gpu_get_power_limit(struct anarchy_device *adev, u32 *limit);
int anarchy_gpu_power_down(struct anarchy_device *adev);

#endif /* ANARCHY_GPU_POWER_H */
