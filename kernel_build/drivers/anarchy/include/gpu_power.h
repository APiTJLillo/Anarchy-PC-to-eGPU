#ifndef ANARCHY_GPU_POWER_H
#define ANARCHY_GPU_POWER_H

#include "types.h"

/* GPU Power Management Functions */
int anarchy_gpu_power_up(struct anarchy_device *adev);
int anarchy_gpu_power_down(struct anarchy_device *adev);

#endif /* ANARCHY_GPU_POWER_H */
