Code Fix for Anarchy-PC-to-eGPU Thunderbolt Bandwidth API Issue
Problem Description
The build error occurs in thunderbolt_utils.h where there's an implicit declaration of function tb_port_bandwidth(). This function appears to be called within the tb_port_get_bandwidth() wrapper function, but it doesn't exist in the current Linux kernel API.
Error message:
/home/tj/Documents/Anarchy-PC-to-eGPU/src/kernel/include/thunderbolt_utils.h: In function 'tb_port_get_bandwidth':
/home/tj/Documents/Anarchy-PC-to-eGPU/src/kernel/include/thunderbolt_utils.h:14:12: error: implicit declaration of function 'tb_port_bandwidth'; did you mean 'tb_port_get_bandwidth'? [-Wimplicit-function-declaration]
   14 |     return tb_port_bandwidth(port);
      |            ^~~~~~~~~~~~~~~~~
      |            tb_port_get_bandwidth
Solution
Based on my analysis of the Linux kernel source code, the tb_port_bandwidth() function doesn't exist in the current kernel API. Instead, we need to implement our own bandwidth calculation function based on the port's link speed and width.
Fix for thunderbolt_utils.h
Replace the current implementation of tb_port_get_bandwidth() with a proper implementation that calculates the bandwidth based on the port's link speed and width:
c
#ifndef _ANARCHY_THUNDERBOLT_UTILS_H_
#define _ANARCHY_THUNDERBOLT_UTILS_H_

#include <linux/types.h>
#include <linux/thunderbolt.h>

/* Get the current bandwidth available on a Thunderbolt port in Gbps */
static inline u32 tb_port_get_bandwidth(struct tb_port *port)
{
    u32 bandwidth = 0;
    
    if (!port)
        return 0;
    
    /* 
     * Calculate bandwidth based on link speed and width
     * These values are approximate and may need adjustment
     * based on your specific hardware
     */
    switch (port->config.type) {
    case TB_TYPE_PORT:
        /* For Thunderbolt 3/USB4 ports */
        if (port->config.max_link_speed == 20) /* 20 Gbps (TB3) */
            bandwidth = 20;
        else if (port->config.max_link_speed == 40) /* 40 Gbps (TB3/USB4) */
            bandwidth = 40;
        else if (port->config.max_link_speed == 80) /* 80 Gbps (USB4 v2) */
            bandwidth = 80;
        else
            bandwidth = 10; /* Default to 10 Gbps */
        
        /* Adjust for link width if available */
        if (port->config.max_link_width == TB_LINK_WIDTH_DUAL)
            bandwidth *= 2;
        else if (port->config.max_link_width == TB_LINK_WIDTH_ASYM_TX ||
                 port->config.max_link_width == TB_LINK_WIDTH_ASYM_RX)
            bandwidth = (bandwidth * 3) / 2; /* 1.5x for asymmetric links */
        break;
        
    case TB_TYPE_DP_HDMI_IN:
    case TB_TYPE_DP_HDMI_OUT:
        /* DisplayPort typically uses fixed bandwidth */
        bandwidth = 32; /* 32 Gbps for DP */
        break;
        
    case TB_TYPE_USB3_DOWN:
    case TB_TYPE_USB3_UP:
        /* USB 3.x ports */
        bandwidth = 10; /* 10 Gbps for USB 3.2 Gen 2 */
        break;
        
    default:
        bandwidth = 0;
        break;
    }
    
    return bandwidth;
}

#endif /* _ANARCHY_THUNDERBOLT_UTILS_H_ */
Alternative Solution
If the above solution doesn't work because the port configuration structure doesn't have the fields we're trying to access, here's a simpler alternative that returns reasonable default values:
c
#ifndef _ANARCHY_THUNDERBOLT_UTILS_H_
#define _ANARCHY_THUNDERBOLT_UTILS_H_

#include <linux/types.h>
#include <linux/thunderbolt.h>

/* Get the current bandwidth available on a Thunderbolt port in Gbps */
static inline u32 tb_port_get_bandwidth(struct tb_port *port)
{
    if (!port)
        return 0;
    
    /* Return a reasonable default value based on port type */
    if (tb_port_is_null(port))
        return 0;
    
    /* For USB4/Thunderbolt ports, assume 40 Gbps */
    return 40;
}

#endif /* _ANARCHY_THUNDERBOLT_UTILS_H_ */
Implementation Notes
The first solution attempts to calculate bandwidth based on the port's link speed and width, which is more accurate but depends on these fields being available in the port configuration structure.
The alternative solution provides a simpler approach by returning reasonable default values, which may be sufficient for your needs if you just need a rough estimate of bandwidth.
You may need to adjust the bandwidth values based on your specific hardware and requirements.
If you need more accurate bandwidth calculations, you might need to implement a more sophisticated approach that takes into account the actual link configuration, negotiated speeds, and other factors.
Testing
After implementing either solution:
Rebuild the module with:
bash
cd /path/to/Anarchy-PC-to-eGPU/src/kernel && make clean && make
Check if the compilation error is resolved.
Test the bandwidth calculation by adding debug prints in your code:
c
pr_info("Port %d bandwidth: %u Gbps\n", port->port, tb_port_get_bandwidth(port));
Load the module and check the kernel logs to verify the bandwidth values are reasonable.