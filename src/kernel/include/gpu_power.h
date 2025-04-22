#ifndef ANARCHY_GPU_POWER_H
#define ANARCHY_GPU_POWER_H

#include "anarchy_device.h"

/* GPU power management functions */
int anarchy_gpu_power_up(struct anarchy_device *adev);
void anarchy_gpu_power_down(struct anarchy_device *adev);

/* GPU thermal control functions */
int anarchy_gpu_set_fan_speed(struct anarchy_device *adev, unsigned int speed);
int anarchy_gpu_set_power_limit(struct anarchy_device *adev, unsigned int limit);

#endif /* ANARCHY_GPU_POWER_H */
