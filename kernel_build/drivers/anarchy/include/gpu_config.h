#ifndef ANARCHY_GPU_CONFIG_H
#define ANARCHY_GPU_CONFIG_H

#include <linux/types.h>

/* MMIO Register Offsets */
#define TEMP_OFFSET          0x1000  /* Temperature sensor register */
#define POWER_OFFSET         0x1004  /* Power consumption register */
#define UTIL_OFFSET         0x1008  /* GPU utilization register */
#define VRAM_UTIL_OFFSET    0x100C  /* VRAM utilization register */
#define PCIE_BW_OFFSET      0x1010  /* PCIe bandwidth register */

/* Fan Configuration */
#define FAN_SPEED_DEFAULT  30  /* Default fan speed percentage */

/* ThinkPad P16 Specific Thermal Configuration */
#define P16_FAN_MIN_SPEED    30  /* 30% minimum */
#define P16_FAN_MAX_SPEED    100 /* 100% maximum */
#define P16_TEMP_TARGET      75  /* 75°C target */
#define P16_TEMP_CRITICAL    87  /* 87°C critical */

/* GPU Thermal Configuration */
#define GPU_THERMAL_TARGET P16_TEMP_TARGET  /* Target temperature in Celsius */
#define GPU_THERMAL_CRIT  85  /* Critical temperature in Celsius */
#define FAN_SPEED_MIN    20  /* Minimum fan speed percentage */
#define FAN_SPEED_MAX   100  /* Maximum fan speed percentage */

/* GPU Power Configuration */
#define GPU_POWER_DEFAULT 175  /* Default power limit in watts */
#define GPU_POWER_MIN    150  /* Minimum power limit in watts */
#define GPU_POWER_MAX    250  /* Maximum power limit in watts */

/* GPU Clock Configuration */
#define GPU_CLOCK_BASE   1000  /* Base clock in MHz */
#define GPU_CLOCK_BOOST  1800  /* Boost clock in MHz */
#define MEM_CLOCK_BASE   7000  /* Base memory clock in MHz */

/* GPU configuration structure */
struct gpu_config {
    /* Core configuration */
    u32 core_clock;        /* Current core clock in MHz */
    u32 memory_clock;      /* Current memory clock in MHz */
    u32 core_voltage;      /* Core voltage in mV */
    u32 memory_voltage;    /* Memory voltage in mV */
    
    /* Clock limits */
    u32 base_clock;        /* Base core clock in MHz */
    u32 boost_clock;       /* Maximum boost clock in MHz */
    u32 mem_clock;         /* Memory clock in MHz */
    
    /* Features */
    bool rtx_enabled;      /* RTX feature enabled */
    bool dlss_enabled;     /* DLSS feature enabled */
    bool cuda_enabled;     /* CUDA compute enabled */
    
    /* Memory configuration */
    u64 vram_size;        /* Total VRAM in bytes */
    u32 memory_type;      /* Memory type (GDDR6X) */
    u32 memory_bus_width; /* Memory bus width */
    
    /* Performance characteristics */
    u32 cuda_cores;       /* Number of CUDA cores */
    u32 tensor_cores;     /* Number of tensor cores */
    u32 rt_cores;        /* Number of RT cores */
};

#endif /* ANARCHY_GPU_CONFIG_H */
