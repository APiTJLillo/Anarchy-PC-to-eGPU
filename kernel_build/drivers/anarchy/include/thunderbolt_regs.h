#ifndef ANARCHY_THUNDERBOLT_REGS_H
#define ANARCHY_THUNDERBOLT_REGS_H

#include <linux/bits.h>

/* Thunderbolt controller register offsets */
#define TB_CONTROL        0x0000  /* Main control register */
#define TB_RESET_CTRL    0x0004  /* Controller reset register */
#define TB_POWER_CTRL    0x0008  /* Power control register */
#define TB_STATUS        0x000C  /* Status register */

/* Control register bit definitions */
#define TB_CONTROL_ENABLE        BIT(0)  /* Enable controller */
#define TB_CONTROL_INT_ENABLE    BIT(1)  /* Enable interrupts */
#define TB_CONTROL_POWER_MGMT    BIT(2)  /* Enable power management */
#define TB_CONTROL_SAFE_MODE     BIT(3)  /* Enable safe mode */

/* Status register bit definitions */
#define TB_STATUS_READY         BIT(0)  /* Controller is ready */
#define TB_STATUS_ERROR         BIT(1)  /* Error condition present */
#define TB_STATUS_POWER_GOOD    BIT(2)  /* Power is stable */
#define TB_STATUS_LINK_UP       BIT(3)  /* Link is established */

/* Error codes */
#define TB_ERR_TIMEOUT         -ETIMEDOUT
#define TB_ERR_INVALID_STATE   -EINVAL
#define TB_ERR_NO_DEVICE      -ENODEV
#define TB_ERR_IO             -EIO

#endif /* ANARCHY_THUNDERBOLT_REGS_H */
