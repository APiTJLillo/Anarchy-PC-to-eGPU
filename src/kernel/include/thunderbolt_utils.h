#ifndef _ANARCHY_THUNDERBOLT_UTILS_H_
#define _ANARCHY_THUNDERBOLT_UTILS_H_

#include <linux/types.h>
#include <linux/thunderbolt.h>

/* Get the current bandwidth available on a Thunderbolt port in Gbps.
 * This implementation provides a conservative default bandwidth value
 * that should work for most USB4/Thunderbolt 3/4 devices. For more
 * precise bandwidth detection, the implementation could be enhanced
 * to probe the actual hardware capabilities.
 */
static inline u32 tb_port_get_bandwidth(struct tb_port *port)
{
    /* Return 0 for invalid ports */
    if (!port)
        return 0;

    /* Default to 40 Gbps which is the standard bandwidth
     * for Thunderbolt 3/4 and USB4 ports */
    return 40;
}

#endif /* _ANARCHY_THUNDERBOLT_UTILS_H_ */ 