#ifndef ANARCHY_THUNDERBOLT_REGS_H
#define ANARCHY_THUNDERBOLT_REGS_H

/* Thunderbolt register offsets */
#define TB_CONTROL          0x0000
#define TB_STATUS          0x0004
#define TB_INTR_STATUS     0x0008
#define TB_INTR_MASK       0x000C
#define TB_CONFIG          0x0010
#define TB_LINK_STATUS     0x0014
#define TB_ERROR_STATUS    0x0018
#define TB_ERROR_MASK      0x001C
#define TB_RESET_CTRL      0x0020
#define TB_POWER_CTRL      0x0024

/* Control register bits */
#define TB_CONTROL_ENABLE  (1 << 0)
#define TB_CONTROL_RESET   (1 << 1)
#define TB_CONTROL_PWRDN   (1 << 2)

/* Status register bits */
#define TB_STATUS_READY    (1 << 0)
#define TB_STATUS_LINK_UP  (1 << 1)
#define TB_STATUS_ERROR    (1 << 2)

/* Function declarations */
u32 tb_read32(struct anarchy_device *adev, u32 reg);
void tb_write32(struct anarchy_device *adev, u32 reg, u32 val);
int anarchy_tb_init(struct anarchy_device *adev);
void anarchy_tb_fini(struct anarchy_device *adev);

#endif /* ANARCHY_THUNDERBOLT_REGS_H */