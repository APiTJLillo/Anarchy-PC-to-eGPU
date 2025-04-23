#include <linux/module.h>
#include <linux/thunderbolt.h>
#include "include/thunderbolt_driver.h"
#include "include/service_probe.h"
#include "include/service_pm.h"

static const struct tb_service_id anarchy_service_ids[] = {
    {
        .match_flags = TBSVC_MATCH_PROTOCOL_KEY,
        .protocol_key = 0x42  /* Match ANARCHY_PROTOCOL_KEY */
    },
    { }  /* Sentinel */
};

struct tb_service_driver anarchy_thunderbolt_driver = {
    .driver = {
        .name = "anarchy_thunderbolt",
        .pm = &anarchy_service_pm,
    },
    .probe = anarchy_service_probe,
    .remove = anarchy_service_remove,
    .id_table = anarchy_service_ids,
};
EXPORT_SYMBOL_GPL(anarchy_thunderbolt_driver);

int tb_service_driver_register(struct tb_service_driver *driver)
{
    return driver_register(&driver->driver);
}
EXPORT_SYMBOL_GPL(tb_service_driver_register);

void tb_service_driver_unregister(struct tb_service_driver *driver)
{
    driver_unregister(&driver->driver);
}
EXPORT_SYMBOL_GPL(tb_service_driver_unregister);
