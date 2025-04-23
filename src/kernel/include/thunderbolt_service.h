#ifndef ANARCHY_THUNDERBOLT_SERVICE_H
#define ANARCHY_THUNDERBOLT_SERVICE_H

#include <linux/types.h>
#include <linux/thunderbolt.h>

/* Event handler type */
typedef void (*tb_service_event_handler)(struct tb_service *service, void *data);

/* Handler registration functions */
int tb_service_register_handler(struct tb_service *service, tb_service_event_handler handler);
void tb_service_unregister_handler(struct tb_service *service);

/* Controller management functions */
int tb_service_reset_controller(struct tb_service *service);
int tb_service_power_cycle_controller(struct tb_service *service);
int tb_service_get_controller_status(struct tb_service *service, u32 *status);

#endif /* ANARCHY_THUNDERBOLT_SERVICE_H */
