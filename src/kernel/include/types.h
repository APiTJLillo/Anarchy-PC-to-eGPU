#ifndef ANARCHY_TYPES_H
#define ANARCHY_TYPES_H

#include <linux/types.h>

/* Forward declarations */
struct anarchy_device;
struct anarchy_ring;
struct anarchy_transfer;
struct tb_service;
struct tb_service_config;
struct game_compat_layer;
struct game_memory_region;
struct command_processor;
struct perf_monitor;
struct usb4_device_info;

/* Basic Constants */
#define ANARCHY_VERSION "1.0"
#define ANARCHY_NUM_BUFFERS 32

/* Device Constants */
#define NVIDIA_VENDOR_ID           0x10DE
#define RTX_4090_MOBILE_DEVICE_ID 0x2407
#define LENOVO_VENDOR_ID          0x17AA
#define LENOVO_P16_SUBSYS_ID      0x22FB

/* Memory Constants */
#define VRAM_SIZE (16ULL * 1024 * 1024 * 1024)  /* 16GB */
#define FB_SIZE   VRAM_SIZE
#define MMIO_SIZE (256 * 1024 * 1024)           /* 256MB */

/* Service Protocol */
#define ANARCHY_SERVICE_PROTOCOL_KEY     0x1
#define ANARCHY_SERVICE_PROTOCOL_ID      0x1
#define ANARCHY_PROTOCOL_VERSION         1

#endif /* ANARCHY_TYPES_H */
