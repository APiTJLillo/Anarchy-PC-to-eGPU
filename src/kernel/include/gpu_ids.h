#ifndef ANARCHY_GPU_IDS_H
#define ANARCHY_GPU_IDS_H

/* NVIDIA Mobile RTX 4090 identifiers */
#define NVIDIA_VENDOR_ID               0x10DE
#define RTX_4090_MOBILE_DEVICE_ID     0x27B0  /* Mobile RTX 4090 */
#define LENOVO_VENDOR_ID              0x17AA
#define LENOVO_P16_SUBSYS_ID          0x22FB  /* Lenovo P16 specific ID */

/* Memory configuration */
#define VRAM_SIZE                     (16ULL * 1024 * 1024 * 1024)  /* 16GB */
#define MMIO_SIZE                     (16 * 1024 * 1024)            /* 16MB */
#define FB_SIZE                       VRAM_SIZE                      /* Full VRAM size */

/* Power and thermal configuration */
#define DEFAULT_POWER_LIMIT           175     /* 175W default TGP */
#define MIN_POWER_LIMIT              150     /* 150W minimum */
#define MAX_POWER_LIMIT              250     /* 250W maximum */
#define DEFAULT_TEMP_TARGET          80      /* 80°C default */
#define DEFAULT_TEMP_LIMIT           87      /* 87°C limit */

/* Clock specifications */
#define BASE_CORE_CLOCK              1395    /* 1395 MHz base */
#define BOOST_CORE_CLOCK             2040    /* 2040 MHz boost */
#define MEMORY_CLOCK                 21000   /* 21 Gbps effective */

#endif /* ANARCHY_GPU_IDS_H */
