#ifndef ANARCHY_HOTPLUG_H
#define ANARCHY_HOTPLUG_H

#include <linux/types.h>
#include <linux/thunderbolt.h>
#include "anarchy_device.h"
#include "pcie_types.h"

/* Thunderbolt service callbacks */
typedef void (*tb_service_event_handler)(struct tb_service *service, void *data);

/* Thunderbolt service functions */
int tb_service_register_handler(struct tb_service *service, tb_service_event_handler handler);
void tb_service_unregister_handler(struct tb_service *service);
void tb_service_set_drvdata(struct tb_service *service, void *data);
void *tb_service_get_drvdata(struct tb_service *service);

/* Hotplug initialization */
int anarchy_hotplug_init(struct anarchy_device *adev);
void anarchy_hotplug_exit(struct anarchy_device *adev);

/* Device state management */
int anarchy_pcie_enable(struct pcie_state *pcie);
void anarchy_pcie_disable(struct pcie_state *pcie);
int anarchy_usb4_init_device(struct anarchy_device *adev);

#endif /* ANARCHY_HOTPLUG_H */
