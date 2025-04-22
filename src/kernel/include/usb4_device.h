#ifndef ANARCHY_USB4_DEVICE_H
#define ANARCHY_USB4_DEVICE_H

#include <linux/types.h>

/* USB4 device types */
enum usb4_device_type {
    USB4_DEV_UNKNOWN = 0,
    USB4_DEV_HOST_ROUTER,
    USB4_DEV_PCIE_DOWN,
    USB4_DEV_DEVICE_ROUTER,
    USB4_DEV_DP_ADAPTER,
    USB4_DEV_EGPU_ADAPTER,
    USB4_DEV_STORAGE,
    USB4_DEV_DOCK,
    USB4_DEV_MAX
};

/* USB4 device string functions */
const char *anarchy_usb4_get_string(int index);

#endif /* ANARCHY_USB4_DEVICE_H */
