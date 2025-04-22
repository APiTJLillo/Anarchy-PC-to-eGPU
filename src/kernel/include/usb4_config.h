#ifndef ANARCHY_USB4_CONFIG_H
#define ANARCHY_USB4_CONFIG_H

#include <linux/types.h>
#include "forward.h"

/* USB4 Protocol Configuration */
#define USB4_MIN_BANDWIDTH     20000  /* 20 Gbps minimum */
#define USB4_MAX_BANDWIDTH     40000  /* 40 Gbps maximum */
#define USB4_MIN_LANES         2      /* Minimum 2 lanes */
#define USB4_MAX_LANES         4      /* Maximum 4 lanes */
#define USB4_RETRY_COUNT       3      /* Connection retry count */
#define USB4_TIMEOUT_MS        5000   /* Connection timeout */

/* USB4 Device Info */
struct usb4_device_info {
    struct anarchy_device *adev;
    unsigned int num_lanes;
    unsigned int link_rate;  /* in Gbps */
    unsigned int current_bandwidth;
    bool dp_tunnel_active;
    bool pcie_tunnel_active;
    bool is_stable;
    unsigned long last_connect;
    unsigned long last_disconnect;
    unsigned int error_count;
};

/* USB4 Management */
int anarchy_usb4_init(struct anarchy_device *adev);
void anarchy_usb4_exit(struct anarchy_device *adev);
int anarchy_usb4_check_connection(struct anarchy_device *adev);
int anarchy_usb4_configure_tunneling(struct anarchy_device *adev);

#endif /* ANARCHY_USB4_CONFIG_H */
