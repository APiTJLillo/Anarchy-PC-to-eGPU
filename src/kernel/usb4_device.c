#include "include/usb4_config.h"

/* Get device string by index */
const char *anarchy_usb4_get_string(int index)
{
    if (index < 0 || index >= ARRAY_SIZE(device_strings))
        return NULL;
    return device_strings[index];
}
EXPORT_SYMBOL_GPL(anarchy_usb4_get_string);
