#include <linux/module.h>
#include <linux/device.h>
#include "include/thunderbolt_driver.h"

static const struct tb_service_id anarchy_service_ids[] = {
    {
        .match_flags = TBSVC_MATCH_PROTOCOL_KEY,
        .protocol_key = 0x42,  /* Match ANARCHY_PROTOCOL_KEY */
        .protocol_id = 1,
        .protocol_version = 1
    },
    { }  /* Terminating entry */
};
MODULE_DEVICE_TABLE(tbsvc, anarchy_service_ids);

/**
 * tb_service_driver_register - Register a Thunderbolt service driver
 * @driver: Driver to register
 *
 * Registers a Thunderbolt service driver with proper error checking.
 */
int tb_service_driver_register(struct tb_service_driver *driver)
{
    if (!driver)
        return -EINVAL;
    if (!driver->driver.name || !driver->id_table)
        return -EINVAL;
    
    driver->driver.bus = &tb_bus_type;
    return driver_register(&driver->driver);
}
EXPORT_SYMBOL_GPL(tb_service_driver_register);

/**
 * tb_service_driver_unregister - Unregister a Thunderbolt service driver
 * @driver: Driver to unregister
 *
 * Unregisters a previously registered Thunderbolt service driver.
 */
void tb_service_driver_unregister(struct tb_service_driver *driver)
{
    if (!driver)
        return;
    driver_unregister(&driver->driver);
}
EXPORT_SYMBOL_GPL(tb_service_driver_unregister);

/* Export the table for module autoloading */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("Thunderbolt service driver for eGPU support");
MODULE_VERSION("1.0");
