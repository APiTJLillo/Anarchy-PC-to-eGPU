#ifndef ANARCHY_THUNDERBOLT_SERVICE_H
#define ANARCHY_THUNDERBOLT_SERVICE_H

#include <linux/thunderbolt.h>

/* Thunderbolt service callbacks */
typedef void (*tb_service_event_handler)(struct tb_service *service, void *data);

/* Thunderbolt service functions */
int tb_service_register_handler(struct tb_service *service, tb_service_event_handler handler);
void tb_service_unregister_handler(struct tb_service *service);
void tb_service_set_drvdata(struct tb_service *service, void *data);

/* These functions are already defined in linux/thunderbolt.h, so we don't declare them here */
/* void *tb_service_get_drvdata(const struct tb_service *service); */

/* Service state management */
int tb_service_reset(struct tb_service *service);
int tb_service_power_cycle(struct tb_service *service);
int tb_service_get_status(struct tb_service *service, u32 *status);

#endif /* ANARCHY_THUNDERBOLT_SERVICE_H */
