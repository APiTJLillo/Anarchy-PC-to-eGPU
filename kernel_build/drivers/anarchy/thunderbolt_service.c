#include <linux/module.h>
#include <linux/delay.h>
#include <linux/thunderbolt.h>
#include "include/thunderbolt_service.h"
#include "include/thunderbolt_regs.h"

struct tb_private {
    void __iomem *mmio_base;
    tb_service_event_handler handler;
};

/* Service event handler registration */
int tb_service_register_handler(struct tb_service *service, tb_service_event_handler handler)
{
    struct tb_private *priv = tb_service_get_drvdata(service);
    if (!service || !handler || !priv)
        return -EINVAL;

    priv->handler = handler;
    return 0;
}
EXPORT_SYMBOL_GPL(tb_service_register_handler);

void tb_service_unregister_handler(struct tb_service *service)
{
    struct tb_private *priv = tb_service_get_drvdata(service);
    if (!service || !priv)
        return;

    priv->handler = NULL;
}
EXPORT_SYMBOL_GPL(tb_service_unregister_handler);

/* Custom thunderbolt service functions */
int tb_service_reset_controller(struct tb_service *service)
{
    struct tb_private *priv = tb_service_get_drvdata(service);
    if (!priv || !priv->mmio_base)
        return -EINVAL;

    /* Issue controller reset */
    writel(1, priv->mmio_base + TB_RESET_CTRL);
    msleep(100); /* Wait for reset to complete */
    
    return 0;
}
EXPORT_SYMBOL_GPL(tb_service_reset_controller);

int tb_service_power_cycle_controller(struct tb_service *service)
{
    struct tb_private *priv = tb_service_get_drvdata(service);
    if (!priv || !priv->mmio_base)
        return -EINVAL;

    /* Power down */
    writel(0, priv->mmio_base + TB_POWER_CTRL);
    msleep(100);
    
    /* Power up */
    writel(1, priv->mmio_base + TB_POWER_CTRL);
    msleep(200); /* Wait for power stabilization */

    return 0;
}
EXPORT_SYMBOL_GPL(tb_service_power_cycle_controller);

int tb_service_get_controller_status(struct tb_service *service, u32 *status)
{
    struct tb_private *priv = tb_service_get_drvdata(service);
    if (!priv || !priv->mmio_base || !status)
        return -EINVAL;

    *status = readl(priv->mmio_base + TB_STATUS);
    return 0;
}
EXPORT_SYMBOL_GPL(tb_service_get_controller_status);
