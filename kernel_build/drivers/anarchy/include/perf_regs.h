#ifndef ANARCHY_PERF_REGS_H
#define ANARCHY_PERF_REGS_H

#include "gpu_offsets.h"

/* Performance thresholds - keep these in perf_regs.h */
#define GPU_TEMP_THRESHOLD    85  /* Celsius */
#define MEM_TEMP_THRESHOLD    95  /* Celsius */
#define FAN_SPEED_THRESHOLD   90  /* Percent */
#define UTIL_THRESHOLD        95  /* Percent */

/* PCIe monitoring registers */
#define PCIE_BANDWIDTH_OFFSET 0x5000
#define PCIE_UTIL_OFFSET     0x5004
#define PCIE_ERROR_OFFSET    0x5008

/* Performance thresholds */
#define GPU_TEMP_THRESHOLD    85  /* Celsius */
#define MEM_TEMP_THRESHOLD    95  /* Celsius */
#define FAN_SPEED_THRESHOLD   90  /* Percent */
#define UTIL_THRESHOLD        95  /* Percent */

#endif /* ANARCHY_PERF_REGS_H */
