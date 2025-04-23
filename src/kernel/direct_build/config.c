#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/json.h>

#include "anarchy-config.h"
#include "anarchy-debug.h"

/* Default configuration values */
static const struct anarchy_preferences default_prefs = {
    .auto_connect = true,
    .auto_power_management = true,
    .persistent_monitoring = true,
    .monitoring_interval = 1000,    /* 1 second */
    .history_retention = 24,        /* 24 hours */
    .default_profile = "balanced",
    .notifications_enabled = true,
    .log_level = 3,                /* INFO level */
};

static const struct anarchy_thresholds default_thresholds = {
    .temp_warning = 80,            /* 80°C */
    .temp_critical = 90,           /* 90°C */
    .power_warning = 300,          /* 300W */
    .power_critical = 350,         /* 350W */
    .memory_warning = 90,          /* 90% */
    .memory_critical = 95,         /* 95% */
    .fan_speed_warning = 80,       /* 80% */
    .fan_speed_critical = 90,      /* 90% */
    .pcie_error_threshold = 10,    /* 10 errors */
    .dma_error_threshold = 5,      /* 5 errors */
};

/* Default performance profiles */
static const struct anarchy_perf_profile default_profiles[] = {
    {
        .name = "power_saver",
        .power_limit = 250,        /* 250W */
        .temp_target = 75,         /* 75°C */
        .fan_speed_min = 30,       /* 30% */
        .fan_speed_max = 70,       /* 70% */
        .core_clock_offset = -100,  /* -100 MHz */
        .mem_clock_offset = 0,
        .adaptive_power = true,
    },
    {
        .name = "balanced",
        .power_limit = 300,        /* 300W */
        .temp_target = 80,         /* 80°C */
        .fan_speed_min = 40,       /* 40% */
        .fan_speed_max = 80,       /* 80% */
        .core_clock_offset = 0,
        .mem_clock_offset = 0,
        .adaptive_power = true,
    },
    {
        .name = "performance",
        .power_limit = 350,        /* 350W */
        .temp_target = 85,         /* 85°C */
        .fan_speed_min = 50,       /* 50% */
        .fan_speed_max = 100,      /* 100% */
        .core_clock_offset = 100,   /* +100 MHz */
        .mem_clock_offset = 200,    /* +200 MHz */
        .adaptive_power = false,
    },
};

/**
 * anarchy_config_init - Initialize configuration management
 * @adev: Anarchy device structure
 *
 * This function initializes the configuration management system.
 */
int anarchy_config_init(struct anarchy_device *adev)
{
    int ret;

    if (!adev)
        return -EINVAL;

    /* Initialize configuration structure */
    mutex_init(&adev->config.lock);
    adev->config.config_loaded = false;

    /* Allocate memory for profiles */
    adev->config.profiles = kmalloc(sizeof(default_profiles), GFP_KERNEL);
    if (!adev->config.profiles)
        return -ENOMEM;

    /* Copy default profiles */
    memcpy(adev->config.profiles, default_profiles, sizeof(default_profiles));
    adev->config.num_profiles = ARRAY_SIZE(default_profiles);

    /* Load configuration from disk */
    ret = anarchy_config_load(adev);
    if (ret) {
        /* Use defaults if loading fails */
        dev_warn(adev->dev, "Using default configuration\n");
        adev->config.prefs = default_prefs;
        adev->config.thresholds = default_thresholds;
    }

    return 0;
}

/**
 * anarchy_config_exit - Clean up configuration management
 * @adev: Anarchy device structure
 *
 * This function cleans up the configuration management system.
 */
void anarchy_config_exit(struct anarchy_device *adev)
{
    if (!adev)
        return;

    /* Save current configuration */
    anarchy_config_save(adev);

    /* Free allocated memory */
    kfree(adev->config.profiles);
    adev->config.profiles = NULL;
}

/**
 * anarchy_config_load - Load configuration from disk
 * @adev: Anarchy device structure
 *
 * This function loads the configuration from disk.
 */
int anarchy_config_load(struct anarchy_device *adev)
{
    struct file *file;
    loff_t pos = 0;
    char *buf;
    int ret = 0;

    if (!adev)
        return -EINVAL;

    mutex_lock(&adev->config.lock);

    /* Open configuration file */
    file = filp_open(ANARCHY_CONFIG_FILE, O_RDONLY, 0);
    if (IS_ERR(file)) {
        ret = PTR_ERR(file);
        goto out_unlock;
    }

    /* Allocate buffer for file contents */
    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf) {
        ret = -ENOMEM;
        goto out_close;
    }

    /* Read file contents */
    ret = kernel_read(file, buf, PAGE_SIZE, &pos);
    if (ret < 0)
        goto out_free;

    /* Parse JSON configuration */
    ret = anarchy_config_parse_json(adev, buf, ret);
    if (ret)
        goto out_free;

    adev->config.config_loaded = true;

out_free:
    kfree(buf);
out_close:
    filp_close(file, NULL);
out_unlock:
    mutex_unlock(&adev->config.lock);
    return ret;
}

/**
 * anarchy_config_save - Save configuration to disk
 * @adev: Anarchy device structure
 *
 * This function saves the current configuration to disk.
 */
int anarchy_config_save(struct anarchy_device *adev)
{
    struct file *file;
    loff_t pos = 0;
    char *buf;
    int ret = 0;
    int len;

    if (!adev)
        return -EINVAL;

    mutex_lock(&adev->config.lock);

    /* Create configuration directory if it doesn't exist */
    ret = anarchy_config_ensure_dir();
    if (ret)
        goto out_unlock;

    /* Open configuration file */
    file = filp_open(ANARCHY_CONFIG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (IS_ERR(file)) {
        ret = PTR_ERR(file);
        goto out_unlock;
    }

    /* Allocate buffer for JSON data */
    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf) {
        ret = -ENOMEM;
        goto out_close;
    }

    /* Generate JSON configuration */
    len = anarchy_config_generate_json(adev, buf, PAGE_SIZE);
    if (len < 0) {
        ret = len;
        goto out_free;
    }

    /* Write configuration to file */
    ret = kernel_write(file, buf, len, &pos);

out_free:
    kfree(buf);
out_close:
    filp_close(file, NULL);
out_unlock:
    mutex_unlock(&adev->config.lock);
    return ret;
}

/**
 * anarchy_config_load_profile - Load a performance profile
 * @adev: Anarchy device structure
 * @name: Profile name
 *
 * This function loads and applies a performance profile.
 */
int anarchy_config_load_profile(struct anarchy_device *adev, const char *name)
{
    struct anarchy_perf_profile *profile = NULL;
    int i;

    if (!adev || !name)
        return -EINVAL;

    mutex_lock(&adev->config.lock);

    /* Find profile by name */
    for (i = 0; i < adev->config.num_profiles; i++) {
        if (strcmp(adev->config.profiles[i].name, name) == 0) {
            profile = &adev->config.profiles[i];
            break;
        }
    }

    if (!profile) {
        mutex_unlock(&adev->config.lock);
        return -ENOENT;
    }

    /* Apply profile settings */
    anarchy_config_apply_profile(adev, profile);

    mutex_unlock(&adev->config.lock);
    return 0;
}

/**
 * anarchy_config_save_profile - Save a performance profile
 * @adev: Anarchy device structure
 * @profile: Profile to save
 *
 * This function saves a performance profile.
 */
int anarchy_config_save_profile(struct anarchy_device *adev,
                              const struct anarchy_perf_profile *profile)
{
    struct anarchy_perf_profile *new_profiles;
    int i;

    if (!adev || !profile)
        return -EINVAL;

    if (!anarchy_config_validate_profile(profile))
        return -EINVAL;

    mutex_lock(&adev->config.lock);

    /* Check if profile already exists */
    for (i = 0; i < adev->config.num_profiles; i++) {
        if (strcmp(adev->config.profiles[i].name, profile->name) == 0) {
            /* Update existing profile */
            memcpy(&adev->config.profiles[i], profile,
                  sizeof(struct anarchy_perf_profile));
            mutex_unlock(&adev->config.lock);
            return 0;
        }
    }

    /* Allocate space for new profile */
    new_profiles = krealloc(adev->config.profiles,
                          (adev->config.num_profiles + 1) *
                          sizeof(struct anarchy_perf_profile),
                          GFP_KERNEL);
    if (!new_profiles) {
        mutex_unlock(&adev->config.lock);
        return -ENOMEM;
    }

    /* Add new profile */
    adev->config.profiles = new_profiles;
    memcpy(&adev->config.profiles[adev->config.num_profiles], profile,
           sizeof(struct anarchy_perf_profile));
    adev->config.num_profiles++;

    mutex_unlock(&adev->config.lock);
    return 0;
}

/**
 * anarchy_config_validate_profile - Validate a performance profile
 * @profile: Profile to validate
 *
 * This function validates a performance profile's settings.
 */
bool anarchy_config_validate_profile(const struct anarchy_perf_profile *profile)
{
    if (!profile)
        return false;

    /* Validate profile settings */
    if (profile->power_limit < 100 || profile->power_limit > 450)
        return false;
    if (profile->temp_target < 60 || profile->temp_target > 90)
        return false;
    if (profile->fan_speed_min > profile->fan_speed_max)
        return false;
    if (profile->fan_speed_max > 100)
        return false;
    if (abs(profile->core_clock_offset) > 500)
        return false;
    if (abs(profile->mem_clock_offset) > 1000)
        return false;

    return true;
}

/**
 * anarchy_config_validate_thresholds - Validate monitoring thresholds
 * @thresholds: Thresholds to validate
 *
 * This function validates monitoring threshold settings.
 */
bool anarchy_config_validate_thresholds(const struct anarchy_thresholds *thresholds)
{
    if (!thresholds)
        return false;

    /* Validate threshold settings */
    if (thresholds->temp_warning >= thresholds->temp_critical)
        return false;
    if (thresholds->power_warning >= thresholds->power_critical)
        return false;
    if (thresholds->memory_warning >= thresholds->memory_critical)
        return false;
    if (thresholds->fan_speed_warning >= thresholds->fan_speed_critical)
        return false;

    return true;
}

/**
 * anarchy_config_apply_profile - Apply a performance profile
 * @adev: Anarchy device structure
 * @profile: Profile to apply
 *
 * This function applies the settings from a performance profile.
 */
void anarchy_config_apply_profile(struct anarchy_device *adev,
                                const struct anarchy_perf_profile *profile)
{
    if (!adev || !profile)
        return;

    /* Apply power limit */
    anarchy_gpu_set_power_limit(adev, profile->power_limit);

    /* Apply temperature target */
    anarchy_gpu_set_temp_target(adev, profile->temp_target);

    /* Apply fan speed settings */
    anarchy_gpu_set_fan_speed_range(adev, profile->fan_speed_min,
                                   profile->fan_speed_max);

    /* Apply clock offsets */
    anarchy_gpu_set_clock_offset(adev, profile->core_clock_offset,
                                profile->mem_clock_offset);

    /* Apply power management setting */
    anarchy_gpu_set_adaptive_power(adev, profile->adaptive_power);

    dev_info(adev->dev, "Applied performance profile: %s\n", profile->name);
}

/**
 * anarchy_config_ensure_dir - Ensure configuration directory exists
 *
 * This function creates the configuration directory if it doesn't exist.
 */
static int anarchy_config_ensure_dir(void)
{
    struct path path;
    int ret;

    ret = kern_path(ANARCHY_CONFIG_DIR, LOOKUP_FOLLOW, &path);
    if (ret) {
        /* Directory doesn't exist, create it */
        ret = mkdir_p(ANARCHY_CONFIG_DIR, 0755);
        if (ret)
            return ret;
    } else {
        path_put(&path);
    }

    return 0;
}

/**
 * anarchy_config_get_path - Get configuration directory path
 *
 * This function returns the path to the configuration directory.
 */
const char *anarchy_config_get_path(void)
{
    return ANARCHY_CONFIG_DIR;
} 