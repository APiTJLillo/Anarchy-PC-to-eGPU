#ifndef ANARCHY_GPU_H
#define ANARCHY_GPU_H

/* GPU Register Offsets */
#define GPU_STATUS_REG         0x0000      /* GPU status register */
#define GPU_CTRL_REG          0x0004      /* GPU control register */
#define GPU_INTR_STAT_REG     0x0008      /* Interrupt status register */
#define GPU_INTR_MASK_REG     0x000C      /* Interrupt mask register */
#define GPU_MEM_CTRL_RESET    0x0010      /* Memory controller reset */
#define GPU_ENGINE_CTRL_RESET 0x0014      /* Engine reset control */
#define PCI_POWER_CONTROL     0x0040      /* Power management control */
#define PCI_POWER_STATUS      0x0044      /* Power management status */

/* GPU Status Register Masks */
#define GPU_TIMEOUT_MASK       0x00000001  /* GPU timeout condition */
#define GPU_ECC_ERROR_MASK     0x00000002  /* ECC error detected */
#define GPU_THERMAL_ALERT_MASK 0x00000004  /* Thermal threshold exceeded */
#define GPU_POWER_ALERT_MASK   0x00000008  /* Power threshold exceeded */
#define GPU_MEMORY_ERROR_MASK  0x00000010  /* Memory access error */
#define GPU_ENGINE_FAULT_MASK  0x00000020  /* GPU engine fault */
#define GPU_FATAL_ERROR_MASK   0x00000040  /* Fatal error condition */

/* GPU Engine State Masks */
#define GPU_ENGINE_STATE_MASK  0x00000700  /* Engine state field */
#define GPU_ENGINE_IDLE        0x00000000  /* Engine is idle */
#define GPU_ENGINE_BUSY        0x00000100  /* Engine is busy */
#define GPU_ENGINE_ERROR       0x00000200  /* Engine is in error state */
#define GPU_ENGINE_RESET       0x00000300  /* Engine is in reset */

/* Power Management Masks */
#define GPU_POWER_STATE_MASK   0x00000003  /* Power state field */
#define GPU_POWER_D0           0x00000000  /* Fully operational */
#define GPU_POWER_D1           0x00000001  /* Light sleep */
#define GPU_POWER_D2           0x00000002  /* Deep sleep */
#define GPU_POWER_D3           0x00000003  /* Power off */

/* Reset Control Bits */
#define GPU_MEM_RESET_BIT      0x00000001  /* Memory controller reset */
#define GPU_ENGINE_RESET_BIT   0x00000001  /* Engine reset */

/* GPU Reset Timeouts (in milliseconds) */
#define GPU_RESET_TIMEOUT      5000        /* Maximum time for reset sequence */
#define GPU_LINK_TIMEOUT       1000        /* Maximum time for link training */
#define GPU_INIT_TIMEOUT       2000        /* Maximum time for initialization */

/* GPU Temperature Thresholds (in degrees Celsius) */
#define GPU_TEMP_WARN          80          /* Warning temperature */
#define GPU_TEMP_CRITICAL      90          /* Critical temperature */
#define GPU_TEMP_SHUTDOWN      95          /* Emergency shutdown temperature */

/* GPU Memory Configuration */
#define GPU_MEM_PAGE_SIZE      4096        /* Memory page size */
#define GPU_MEM_MAX_SEGMENTS   256         /* Maximum memory segments */
#define GPU_MEM_MIN_ALLOC      4096        /* Minimum allocation size */

/* Function Declarations */
int anarchy_gpu_init(struct anarchy_device *adev);
void anarchy_gpu_exit(struct anarchy_device *adev);
void anarchy_gpu_handle_error(struct anarchy_device *adev, enum anarchy_gpu_error error);
int anarchy_gpu_reset(struct anarchy_device *adev);
void anarchy_gpu_check_status(struct anarchy_device *adev);

#endif /* ANARCHY_GPU_H */ 