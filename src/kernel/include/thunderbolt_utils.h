#ifndef _ANARCHY_THUNDERBOLT_UTILS_H_
#define _ANARCHY_THUNDERBOLT_UTILS_H_

#include <linux/types.h>
#include <linux/thunderbolt.h>

/* Get the current bandwidth available on a Thunderbolt port */
static inline u32 tb_port_get_bandwidth(struct tb_port *port)
{
    if (!port)
        return 0;
    
    /* Return bandwidth in Gbps */
    return tb_port_bandwidth(port);
}

#endif /* _ANARCHY_THUNDERBOLT_UTILS_H_ */ 