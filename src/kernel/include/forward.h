#ifndef ANARCHY_FORWARD_H
#define ANARCHY_FORWARD_H

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/completion.h>

/* Forward declarations only - no struct definitions */
struct anarchy_device;
struct pci_dev;
struct device;
struct tb_service;
struct tb_ring;
struct perf_state;
struct perf_monitor;
struct gpu_config;
struct gpu_config_ops;
struct power_profile;
struct anarchy_ring;
struct anarchy_transfer;
struct anarchy_pcie_state;
struct tb_service_config;
struct usb4_device_info;
struct game_compat_layer;
struct game_profile;
struct command_processor;
struct bandwidth_config;
struct thermal_profile;
struct gpu_emu_config;
struct gpu_emu_interface;
struct game_memory_region;

#endif /* ANARCHY_FORWARD_H */
