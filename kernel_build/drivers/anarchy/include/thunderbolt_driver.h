#ifndef ANARCHY_THUNDERBOLT_DRIVER_H
#define ANARCHY_THUNDERBOLT_DRIVER_H

#include <linux/thunderbolt.h>
#include <linux/device.h>

struct tb_service_driver *anarchy_thunderbolt_driver_get(void);

/* Function declarations */
int tb_service_driver_register(struct tb_service_driver *driver);
void tb_service_driver_unregister(struct tb_service_driver *driver);

#endif /* ANARCHY_THUNDERBOLT_DRIVER_H */
