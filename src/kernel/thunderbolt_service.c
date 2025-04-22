#include <linux/module.h>
#include <linux/thunderbolt.h>
#include "include/thunderbolt_service.h"

/* Service event handler registration */
int tb_service_register_handler(struct tb_service *service, tb_service_event_handler handler)
{
    if (!service || !handler)
        return -EINVAL;

    /* Store handler in service private data */
    service->event_handler = handler;
    return 0;
}

void tb_service_unregister_handler(struct tb_service *service)
{
    if (!service)
        return;
    service->event_handler = NULL;
}

/* Service driver data management */
void tb_service_set_drvdata(struct tb_service *service, void *data)
{
    if (!service)
        return;
    service->driver_data = data;
}

void *tb_service_get_drvdata(struct tb_service *service)
{
    if (!service)
        return NULL;
    return service->driver_data;
}

/* Service state management */
int tb_service_reset(struct tb_service *service)
{
    if (!service)
        return -EINVAL;

    /* Issue controller reset */
    writel(1, service->mmio_base + TB_RESET_CTRL);
    msleep(100); /* Wait for reset to complete */
    
    return 0;
}

int tb_service_power_cycle(struct tb_service *service)
{
    if (!service)
        return -EINVAL;

    /* Power down */
    writel(0, service->mmio_base + TB_POWER_CTRL);
    msleep(100);
    
    /* Power up */
    writel(1, service->mmio_base + TB_POWER_CTRL);
    msleep(200); /* Wait for power stabilization */

    return 0;
}

int tb_service_get_status(struct tb_service *service, u32 *status)
{
    if (!service || !status)
        return -EINVAL;

    *status = readl(service->mmio_base + TB_STATUS);
    return 0;
}

EXPORT_SYMBOL_GPL(tb_service_register_handler);
EXPORT_SYMBOL_GPL(tb_service_unregister_handler);
EXPORT_SYMBOL_GPL(tb_service_set_drvdata);
EXPORT_SYMBOL_GPL(tb_service_get_drvdata);
EXPORT_SYMBOL_GPL(tb_service_reset);
EXPORT_SYMBOL_GPL(tb_service_power_cycle);
EXPORT_SYMBOL_GPL(tb_service_get_status);
