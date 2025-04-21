#ifndef ANARCHY_CONFIG_H
#define ANARCHY_CONFIG_H

#include <linux/types.h>
#include <linux/mutex.h>

/* Configuration file paths */
#define ANARCHY_CONFIG_DIR          "/etc/anarchy-egpu"
#define ANARCHY_CONFIG_FILE         ANARCHY_CONFIG_DIR "/config.json"
#define ANARCHY_PROFILES_DIR        ANARCHY_CONFIG_DIR "/profiles"
#define ANARCHY_THRESHOLDS_FILE     ANARCHY_CONFIG_DIR "/thresholds.json"

/* Performance profile settings */
struct anarchy_perf_profile {
    char name[32];                  /* Profile name */
    u32 power_limit;               /* Power limit in watts */
    u32 temp_target;               /* Temperature target in celsius */
    u32 fan_speed_min;             /* Minimum fan speed percentage */
    u32 fan_speed_max;             /* Maximum fan speed percentage */
    u32 core_clock_offset;         /* Core clock offset in MHz */
    u32 mem_clock_offset;          /* Memory clock offset in MHz */
    bool adaptive_power;           /* Enable adaptive power management */
};

/* Monitoring threshold settings */
struct anarchy_thresholds {
    u32 temp_warning;              /* Temperature warning threshold */
    u32 temp_critical;             /* Temperature critical threshold */
    u32 power_warning;             /* Power consumption warning threshold */
    u32 power_critical;            /* Power consumption critical threshold */
    u32 memory_warning;            /* Memory usage warning threshold */
    u32 memory_critical;           /* Memory usage critical threshold */
    u32 fan_speed_warning;         /* Fan speed warning threshold */
    u32 fan_speed_critical;        /* Fan speed critical threshold */
    u32 pcie_error_threshold;      /* PCIe error count threshold */
    u32 dma_error_threshold;       /* DMA error count threshold */
};

/* User preferences */
struct anarchy_preferences {
    bool auto_connect;             /* Auto-connect on startup */
    bool auto_power_management;    /* Automatic power management */
    bool persistent_monitoring;    /* Keep monitoring history */
    u32 monitoring_interval;       /* Monitoring update interval in ms */
    u32 history_retention;         /* History retention period in hours */
    char default_profile[32];      /* Default performance profile name */
    bool notifications_enabled;    /* Enable system notifications */
    u32 log_level;                /* Logging verbosity level */
};

/* Main configuration structure */
struct anarchy_config {
    struct mutex lock;             /* Configuration access lock */
    struct anarchy_preferences prefs;
    struct anarchy_thresholds thresholds;
    struct anarchy_perf_profile *profiles;
    u32 num_profiles;
    bool config_loaded;            /* Configuration load status */
};

/* Function declarations */
int anarchy_config_init(struct anarchy_device *adev);
void anarchy_config_exit(struct anarchy_device *adev);
int anarchy_config_load(struct anarchy_device *adev);
int anarchy_config_save(struct anarchy_device *adev);
int anarchy_config_load_profile(struct anarchy_device *adev, const char *name);
int anarchy_config_save_profile(struct anarchy_device *adev, const struct anarchy_perf_profile *profile);
int anarchy_config_update_thresholds(struct anarchy_device *adev, const struct anarchy_thresholds *thresholds);
int anarchy_config_update_preferences(struct anarchy_device *adev, const struct anarchy_preferences *prefs);

/* Helper functions */
const char *anarchy_config_get_path(void);
bool anarchy_config_validate_profile(const struct anarchy_perf_profile *profile);
bool anarchy_config_validate_thresholds(const struct anarchy_thresholds *thresholds);
void anarchy_config_apply_profile(struct anarchy_device *adev, const struct anarchy_perf_profile *profile);

#endif /* ANARCHY_CONFIG_H */ 