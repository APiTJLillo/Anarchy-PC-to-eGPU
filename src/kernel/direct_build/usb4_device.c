#include "include/usb4_config.h"
#include "include/usb4_device.h"
#include <linux/module.h>
#include <linux/kernel.h>

/* USB4 device string descriptors */
static const char *device_strings[] = {
    "Unknown",
    "USB4 Host Router",
    "PCIe Downstream Port",
    "USB4 Device Router",
    "DisplayPort Adapter",
    "eGPU Adapter",
    "Storage Adapter",
    "Dock"
};

const char *anarchy_usb4_get_string(int index)
{
    if (index < 0 || index >= ARRAY_SIZE(device_strings))
        return device_strings[0];  /* Return "Unknown" for invalid indices */
    return device_strings[index];
}

EXPORT_SYMBOL_GPL(anarchy_usb4_get_string);
