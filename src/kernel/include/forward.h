#ifndef ANARCHY_FORWARD_H
#define ANARCHY_FORWARD_H

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/completion.h>

/* Performance monitoring states */
struct perf_state {
    unsigned int gpu_clock;
    unsigned int mem_clock;
    unsigned int power_draw;
    unsigned int temperature;
    unsigned int fan_speed;
    unsigned int gpu_util;
    unsigned int mem_util;
    unsigned int pcie_util;
    unsigned int vram_used;
};

/* Performance monitor structure */
struct perf_monitor {
    struct anarchy_device *adev;
    struct perf_state current_state;
    struct workqueue_struct *wq;
    struct delayed_work update_work;
    spinlock_t lock;
    unsigned int update_interval;
    bool enabled;
};

/* Device structures */
struct anarchy_device;
struct pci_dev;
struct device;
struct tb_service;
struct tb_ring;

/* GPU/Memory structures */
struct gpu_config;
struct gpu_config_ops {
    int (*init)(struct anarchy_device *adev);
    void (*cleanup)(struct anarchy_device *adev);
    int (*set_power_profile)(struct anarchy_device *adev, struct power_profile *profile);
    int (*get_temperature)(struct anarchy_device *adev);
    int (*get_power)(struct anarchy_device *adev);
    int (*get_utilization)(struct anarchy_device *adev);
};

struct power_profile {
    unsigned int fan_speed;      /* Current fan speed percentage */
    unsigned int power_limit;    /* Current power limit in watts */
    bool dynamic_control;        /* Whether dynamic control is enabled */
};

struct ring_frame {
    void *data;
    size_t size;
    u32 flags;
    void *completion;
};
struct anarchy_ring;
struct anarchy_transfer;

/* PCIe/Thunderbolt structures */
struct anarchy_pcie_state;
struct tb_service_config;
struct usb4_device_info;

/* Game/Performance structures */
struct game_compat_layer;
struct game_profile;
struct perf_monitor;
struct command_processor;
struct bandwidth_config;
struct thermal_profile;

/* GPU emulation structures */
struct gpu_emu_config;
struct gpu_emu_interface;
struct game_memory_region;

#endif /* ANARCHY_FORWARD_H */
