#include <linux/module.h>
#include <linux/device.h>
#include "include/thunderbolt_bus.h"

struct bus_type tb_bus_type = {
    .name = "thunderbolt",
};
EXPORT_SYMBOL_GPL(tb_bus_type); 