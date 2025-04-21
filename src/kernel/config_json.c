#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/json.h>

#include "anarchy-config.h"
#include "anarchy-debug.h"

/* Helper functions for JSON parsing */
static int parse_preferences(struct anarchy_device *adev,
                           const struct json_object *obj);
static int parse_thresholds(struct anarchy_device *adev,
                          const struct json_object *obj);
static int parse_profiles(struct anarchy_device *adev,
                         const struct json_array *arr);
static int parse_profile(struct anarchy_perf_profile *profile,
                        const struct json_object *obj);

/* Helper functions for JSON generation */
static int generate_preferences(struct anarchy_device *adev,
                              struct json_object *obj);
static int generate_thresholds(struct anarchy_device *adev,
                             struct json_object *obj);
static int generate_profiles(struct anarchy_device *adev,
                           struct json_array *arr);
static int generate_profile(const struct anarchy_perf_profile *profile,
                          struct json_object *obj);

/**
 * anarchy_config_parse_json - Parse JSON configuration data
 * @adev: Anarchy device structure
 * @buf: Buffer containing JSON data
 * @len: Length of buffer
 *
 * This function parses JSON configuration data and updates device settings.
 */
int anarchy_config_parse_json(struct anarchy_device *adev,
                            const char *buf, size_t len)
{
    struct json_object *root, *obj;
    int ret = 0;

    root = json_parse(buf, len);
    if (!root)
        return -EINVAL;

    /* Parse preferences */
    obj = json_object_get(root, "preferences");
    if (obj) {
        ret = parse_preferences(adev, obj);
        if (ret)
            goto out;
    }

    /* Parse thresholds */
    obj = json_object_get(root, "thresholds");
    if (obj) {
        ret = parse_thresholds(adev, obj);
        if (ret)
            goto out;
    }

    /* Parse profiles */
    obj = json_object_get(root, "profiles");
    if (obj) {
        ret = parse_profiles(adev, json_object_array(obj));
        if (ret)
            goto out;
    }

out:
    json_free(root);
    return ret;
}

/**
 * anarchy_config_generate_json - Generate JSON configuration data
 * @adev: Anarchy device structure
 * @buf: Buffer to store JSON data
 * @size: Size of buffer
 *
 * This function generates JSON configuration data from current settings.
 */
int anarchy_config_generate_json(struct anarchy_device *adev,
                               char *buf, size_t size)
{
    struct json_object *root;
    struct json_object *obj;
    struct json_array *arr;
    int ret;

    root = json_object_create();
    if (!root)
        return -ENOMEM;

    /* Generate preferences */
    obj = json_object_create();
    if (!obj) {
        ret = -ENOMEM;
        goto out_free_root;
    }
    ret = generate_preferences(adev, obj);
    if (ret)
        goto out_free_obj;
    json_object_add(root, "preferences", obj);

    /* Generate thresholds */
    obj = json_object_create();
    if (!obj) {
        ret = -ENOMEM;
        goto out_free_root;
    }
    ret = generate_thresholds(adev, obj);
    if (ret)
        goto out_free_obj;
    json_object_add(root, "thresholds", obj);

    /* Generate profiles */
    arr = json_array_create();
    if (!arr) {
        ret = -ENOMEM;
        goto out_free_root;
    }
    ret = generate_profiles(adev, arr);
    if (ret)
        goto out_free_arr;
    json_object_add_array(root, "profiles", arr);

    /* Generate JSON string */
    ret = json_stringify(root, buf, size);

out_free_arr:
    json_array_free(arr);
out_free_obj:
    json_object_free(obj);
out_free_root:
    json_object_free(root);
    return ret;
}

/* Helper function implementations */

static int parse_preferences(struct anarchy_device *adev,
                           const struct json_object *obj)
{
    struct anarchy_preferences *prefs = &adev->config.prefs;

    /* Parse boolean values */
    prefs->auto_connect = json_object_get_bool(obj, "auto_connect",
                                             default_prefs.auto_connect);
    prefs->auto_power_management = json_object_get_bool(obj, "auto_power_management",
                                                      default_prefs.auto_power_management);
    prefs->persistent_monitoring = json_object_get_bool(obj, "persistent_monitoring",
                                                      default_prefs.persistent_monitoring);
    prefs->notifications_enabled = json_object_get_bool(obj, "notifications_enabled",
                                                      default_prefs.notifications_enabled);

    /* Parse integer values */
    prefs->monitoring_interval = json_object_get_int(obj, "monitoring_interval",
                                                   default_prefs.monitoring_interval);
    prefs->history_retention = json_object_get_int(obj, "history_retention",
                                                 default_prefs.history_retention);
    prefs->log_level = json_object_get_int(obj, "log_level",
                                         default_prefs.log_level);

    /* Parse string values */
    strncpy(prefs->default_profile, json_object_get_string(obj, "default_profile",
                                                          default_prefs.default_profile),
            sizeof(prefs->default_profile) - 1);

    return 0;
}

static int parse_thresholds(struct anarchy_device *adev,
                          const struct json_object *obj)
{
    struct anarchy_thresholds *thresh = &adev->config.thresholds;

    /* Parse temperature thresholds */
    thresh->temp_warning = json_object_get_int(obj, "temp_warning",
                                             default_thresholds.temp_warning);
    thresh->temp_critical = json_object_get_int(obj, "temp_critical",
                                              default_thresholds.temp_critical);

    /* Parse power thresholds */
    thresh->power_warning = json_object_get_int(obj, "power_warning",
                                              default_thresholds.power_warning);
    thresh->power_critical = json_object_get_int(obj, "power_critical",
                                               default_thresholds.power_critical);

    /* Parse memory thresholds */
    thresh->memory_warning = json_object_get_int(obj, "memory_warning",
                                               default_thresholds.memory_warning);
    thresh->memory_critical = json_object_get_int(obj, "memory_critical",
                                                default_thresholds.memory_critical);

    /* Parse fan speed thresholds */
    thresh->fan_speed_warning = json_object_get_int(obj, "fan_speed_warning",
                                                  default_thresholds.fan_speed_warning);
    thresh->fan_speed_critical = json_object_get_int(obj, "fan_speed_critical",
                                                   default_thresholds.fan_speed_critical);

    /* Parse error thresholds */
    thresh->pcie_error_threshold = json_object_get_int(obj, "pcie_error_threshold",
                                                     default_thresholds.pcie_error_threshold);
    thresh->dma_error_threshold = json_object_get_int(obj, "dma_error_threshold",
                                                    default_thresholds.dma_error_threshold);

    return anarchy_config_validate_thresholds(thresh) ? 0 : -EINVAL;
}

static int parse_profiles(struct anarchy_device *adev,
                         const struct json_array *arr)
{
    struct anarchy_perf_profile *new_profiles;
    size_t num_profiles = json_array_size(arr);
    int i, ret;

    if (num_profiles == 0)
        return 0;

    /* Allocate memory for profiles */
    new_profiles = kmalloc(num_profiles * sizeof(struct anarchy_perf_profile),
                          GFP_KERNEL);
    if (!new_profiles)
        return -ENOMEM;

    /* Parse each profile */
    for (i = 0; i < num_profiles; i++) {
        ret = parse_profile(&new_profiles[i],
                          json_array_get_object(arr, i));
        if (ret) {
            kfree(new_profiles);
            return ret;
        }
    }

    /* Replace existing profiles */
    kfree(adev->config.profiles);
    adev->config.profiles = new_profiles;
    adev->config.num_profiles = num_profiles;

    return 0;
}

static int parse_profile(struct anarchy_perf_profile *profile,
                        const struct json_object *obj)
{
    /* Parse profile name */
    strncpy(profile->name, json_object_get_string(obj, "name", ""),
            sizeof(profile->name) - 1);

    /* Parse numeric values */
    profile->power_limit = json_object_get_int(obj, "power_limit", 300);
    profile->temp_target = json_object_get_int(obj, "temp_target", 80);
    profile->fan_speed_min = json_object_get_int(obj, "fan_speed_min", 40);
    profile->fan_speed_max = json_object_get_int(obj, "fan_speed_max", 80);
    profile->core_clock_offset = json_object_get_int(obj, "core_clock_offset", 0);
    profile->mem_clock_offset = json_object_get_int(obj, "mem_clock_offset", 0);

    /* Parse boolean values */
    profile->adaptive_power = json_object_get_bool(obj, "adaptive_power", true);

    return anarchy_config_validate_profile(profile) ? 0 : -EINVAL;
}

static int generate_preferences(struct anarchy_device *adev,
                              struct json_object *obj)
{
    const struct anarchy_preferences *prefs = &adev->config.prefs;

    /* Add boolean values */
    json_object_add_bool(obj, "auto_connect", prefs->auto_connect);
    json_object_add_bool(obj, "auto_power_management", prefs->auto_power_management);
    json_object_add_bool(obj, "persistent_monitoring", prefs->persistent_monitoring);
    json_object_add_bool(obj, "notifications_enabled", prefs->notifications_enabled);

    /* Add integer values */
    json_object_add_int(obj, "monitoring_interval", prefs->monitoring_interval);
    json_object_add_int(obj, "history_retention", prefs->history_retention);
    json_object_add_int(obj, "log_level", prefs->log_level);

    /* Add string values */
    json_object_add_string(obj, "default_profile", prefs->default_profile);

    return 0;
}

static int generate_thresholds(struct anarchy_device *adev,
                             struct json_object *obj)
{
    const struct anarchy_thresholds *thresh = &adev->config.thresholds;

    /* Add temperature thresholds */
    json_object_add_int(obj, "temp_warning", thresh->temp_warning);
    json_object_add_int(obj, "temp_critical", thresh->temp_critical);

    /* Add power thresholds */
    json_object_add_int(obj, "power_warning", thresh->power_warning);
    json_object_add_int(obj, "power_critical", thresh->power_critical);

    /* Add memory thresholds */
    json_object_add_int(obj, "memory_warning", thresh->memory_warning);
    json_object_add_int(obj, "memory_critical", thresh->memory_critical);

    /* Add fan speed thresholds */
    json_object_add_int(obj, "fan_speed_warning", thresh->fan_speed_warning);
    json_object_add_int(obj, "fan_speed_critical", thresh->fan_speed_critical);

    /* Add error thresholds */
    json_object_add_int(obj, "pcie_error_threshold", thresh->pcie_error_threshold);
    json_object_add_int(obj, "dma_error_threshold", thresh->dma_error_threshold);

    return 0;
}

static int generate_profiles(struct anarchy_device *adev,
                           struct json_array *arr)
{
    struct json_object *obj;
    int i, ret;

    for (i = 0; i < adev->config.num_profiles; i++) {
        obj = json_object_create();
        if (!obj)
            return -ENOMEM;

        ret = generate_profile(&adev->config.profiles[i], obj);
        if (ret) {
            json_object_free(obj);
            return ret;
        }

        json_array_add(arr, obj);
    }

    return 0;
}

static int generate_profile(const struct anarchy_perf_profile *profile,
                          struct json_object *obj)
{
    /* Add profile name */
    json_object_add_string(obj, "name", profile->name);

    /* Add numeric values */
    json_object_add_int(obj, "power_limit", profile->power_limit);
    json_object_add_int(obj, "temp_target", profile->temp_target);
    json_object_add_int(obj, "fan_speed_min", profile->fan_speed_min);
    json_object_add_int(obj, "fan_speed_max", profile->fan_speed_max);
    json_object_add_int(obj, "core_clock_offset", profile->core_clock_offset);
    json_object_add_int(obj, "mem_clock_offset", profile->mem_clock_offset);

    /* Add boolean values */
    json_object_add_bool(obj, "adaptive_power", profile->adaptive_power);

    return 0;
} 