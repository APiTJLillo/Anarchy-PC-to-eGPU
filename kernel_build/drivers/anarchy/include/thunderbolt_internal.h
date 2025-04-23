#ifndef ANARCHY_THUNDERBOLT_INTERNAL_H
#define ANARCHY_THUNDERBOLT_INTERNAL_H

#include "types.h"

/* Thunderbolt Register Access Functions */
u32 tb_read32(struct anarchy_device *adev, u32 reg);
void tb_write32(struct anarchy_device *adev, u32 reg, u32 val);

/* Thunderbolt Initialization/Cleanup */
int anarchy_tb_init(struct anarchy_device *adev);
void anarchy_tb_fini(struct anarchy_device *adev);

/* Module Init/Exit */
int anarchy_thunderbolt_init(void);
void anarchy_thunderbolt_cleanup(void);

#endif /* ANARCHY_THUNDERBOLT_INTERNAL_H */
