#ifndef ANARCHY_COMMON_H
#define ANARCHY_COMMON_H

#include <linux/types.h>
#include "forward.h"
#include "types.h"

/* Module versioning */
#define ANARCHY_DRIVER_VERSION "1.0"

/* PCIe configuration */
#define PCIE_GEN4_SPEED    16  /* GT/s */
#define PCIE_MAX_LANES     8   /* x8 for TB4/USB4 */
#define PCIE_MIN_LANES     4   /* Minimum x4 required */

/* Power management */
#define DEFAULT_POWER_LIMIT 175 /* Default 175W TGP */
#define MIN_POWER_LIMIT    150 /* Minimum 150W */
#define MAX_POWER_LIMIT    250 /* Maximum 250W */
#define DEFAULT_FAN_SPEED  50  /* 50% default */

/* Memory configuration */
#define VRAM_SIZE (16ULL * 1024 * 1024 * 1024) /* 16GB VRAM */
#define MMIO_SIZE (256 * 1024 * 1024)          /* 256MB MMIO */

/* DMA/Ring configuration */
#define RING_BUFFER_SIZE    32768  /* 32KB ring buffer */
#define MAX_DMA_CHANNELS    12     /* Maximum 12 DMA channels */
#define MIN_DMA_CHANNELS    4      /* Minimum 4 DMA channels */
#define DMA_BUFFER_SIZE     65536  /* 64KB DMA buffers */

/* Error handling */
#define MAX_RECOVERY_ATTEMPTS  3   /* Maximum recovery attempts */
#define RECOVERY_WAIT_MS      1000 /* 1 second between attempts */

#endif /* ANARCHY_COMMON_H */
