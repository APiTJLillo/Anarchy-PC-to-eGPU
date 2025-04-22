#ifndef ANARCHY_BANDWIDTH_H
#define ANARCHY_BANDWIDTH_H

#include <linux/types.h>
#include "forward.h"

/* Bandwidth configuration structure */
struct bandwidth_config {
    unsigned int current_bw;
    unsigned int required_bw;
    unsigned int available_bw;
    unsigned int min_bw;
    unsigned int max_bw;
    unsigned int optimal_bw;
    bool bw_critical;
    spinlock_t lock;
    unsigned long last_update;
};

/* Bandwidth thresholds */
#define BW_MIN_REQUIRED   20000  /* 20 Gbps minimum */
#define BW_OPTIMAL        32000  /* 32 Gbps optimal */
#define BW_MAX_USB4       40000  /* 40 Gbps max USB4 */
#define BW_CRITICAL_LOW   15000  /* 15 Gbps critical */

/* Bandwidth management functions */
int anarchy_bandwidth_init(struct anarchy_device *adev);
void anarchy_bandwidth_cleanup(struct anarchy_device *adev);
int anarchy_bandwidth_update(struct anarchy_device *adev);
bool anarchy_bandwidth_check_critical(struct anarchy_device *adev);

#endif /* ANARCHY_BANDWIDTH_H */
