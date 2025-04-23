#ifndef ANARCHY_GPU_OFFSETS_H
#define ANARCHY_GPU_OFFSETS_H

/* Power management registers */
#define PWR_CTRL_OFFSET      0x1020
#define PWR_LIMIT_OFFSET     0x1028
#define PWR_STATUS_OFFSET    0x102C
#define FAN_CTRL_OFFSET      0x1040
#define FAN_STATUS_OFFSET    0x1044

/* Clock control registers */
#define CLK_CTRL_OFFSET      0x1100
#define CLK_STATUS_OFFSET    0x1104
#define MEM_CLK_OFFSET       0x1108

/* Temperature sensors */
#define TEMP_OFFSET          0x1000
#define TEMP_STATUS_OFFSET   0x1004

/* Performance monitoring registers */
#define GPU_CLOCK_OFFSET     0x4000
#define MEM_CLOCK_OFFSET     0x4004
#define GPU_UTIL_OFFSET      0x400C
#define MEM_UTIL_OFFSET      0x4010
#define VRAM_USED_OFFSET     0x4014

/* PCIe monitoring registers */
#define PCIE_BANDWIDTH_OFFSET 0x5000
#define PCIE_UTIL_OFFSET     0x5004
#define PCIE_ERROR_OFFSET    0x5008

#endif /* ANARCHY_GPU_OFFSETS_H */
