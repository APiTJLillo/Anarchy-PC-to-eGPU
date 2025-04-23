#ifndef _ANARCHY_BANDWIDTH_H_
#define _ANARCHY_BANDWIDTH_H_

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include "anarchy_device_forward.h"

/* PCIe bandwidth counters */
#define PCIE_RX_COUNTER      0x5000
#define PCIE_TX_COUNTER      0x5004

/* Bandwidth monitoring interval (in milliseconds) */
#define BANDWIDTH_UPDATE_INTERVAL 1000

/* TB4/USB4 bandwidth configuration */
#define TB4_MAX_BANDWIDTH    40000  /* 40 Gbps max */
#define PCIE_X8_BANDWIDTH    16000  /* ~16 GB/s for PCIe 4.0 x8 */
#define MIN_GAMING_BANDWIDTH 10000  /* 10 Gbps minimum for gaming */

/* Bandwidth configuration structure */
struct bandwidth_config {
    u32 current_bandwidth;
    u32 required_bandwidth;
    u32 available_bandwidth;
    bool bandwidth_critical;
    struct workqueue_struct *wq;
    struct delayed_work update_work;
    spinlock_t lock;
    unsigned long last_update;
};

/* Bandwidth management functions */
u32 anarchy_pcie_get_bandwidth_usage(struct anarchy_device *adev);
int init_bandwidth_monitoring(struct anarchy_device *adev);
void cleanup_bandwidth_monitoring(struct anarchy_device *adev);

#endif /* _ANARCHY_BANDWIDTH_H_ */
