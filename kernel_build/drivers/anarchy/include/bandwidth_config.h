#ifndef ANARCHY_BANDWIDTH_CONFIG_H
#define ANARCHY_BANDWIDTH_CONFIG_H

#include <linux/types.h>

/* Bandwidth configuration structure */
struct bandwidth_config {
    u64 available_bandwidth;      /* Current available bandwidth in bytes/sec */
    u64 total_bandwidth;          /* Total link bandwidth in bytes/sec */
    u64 min_bandwidth;           /* Minimum required bandwidth for operation */
    bool bandwidth_critical;      /* Flag indicating if bandwidth is critically low */
    unsigned long last_update;    /* Last bandwidth update timestamp */
    spinlock_t lock;             /* Lock for protecting bandwidth updates */
};

#endif /* ANARCHY_BANDWIDTH_CONFIG_H */
