#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/bits.h>
#include "include/anarchy_device.h"
#include "include/gpu_power.h"
#include "include/power_mgmt.h"
#include "include/gpu_offsets.h"

/* Use power limits from gpu_power.h */
#ifndef GPU_POWER_LIMIT_MIN
#define GPU_POWER_LIMIT_MIN  150
#endif
#ifndef GPU_POWER_LIMIT_MAX
#define GPU_POWER_LIMIT_MAX  250
#endif

/* Clock limits in MHz */
#define MIN_GPU_CLOCK        300
#define MAX_GPU_CLOCK        2640
#define MIN_MEM_CLOCK        810
#define MAX_MEM_CLOCK        21000

static int apply_power_profile(struct anarchy_device *adev, struct power_profile *profile)
{
    u32 reg;

    /* Validate limits */
    if (profile->power_limit < GPU_POWER_LIMIT_MIN || profile->power_limit > GPU_POWER_LIMIT_MAX)
        return -EINVAL;
    if (profile->fan_speed > 100)
        return -EINVAL;

    /* Update power limit */
    writel(profile->power_limit, adev->mmio_base + PWR_LIMIT_OFFSET);
    writel(1, adev->mmio_base + PWR_CTRL_OFFSET); /* Apply changes */
    
    /* Update fan control */
    reg = profile->fan_speed | (profile->dynamic_control ? BIT(31) : 0);
    writel(reg, adev->mmio_base + FAN_CTRL_OFFSET);

    /* Wait for changes to take effect */
    msleep(10);

    /* Verify changes */
    reg = readl(adev->mmio_base + PWR_STATUS_OFFSET);
    if ((reg & 0xFFFF) != profile->power_limit)
        return -EIO;

    return 0;
}

int anarchy_power_init(struct anarchy_device *adev)
{
    struct power_profile profile;
    u32 reg;

    if (!adev)
        return -EINVAL;

    /* Check if power management is supported */
    reg = readl(adev->mmio_base + PWR_CTRL_OFFSET);
    if (!(reg & BIT(31))) {
        dev_err(&adev->pdev->dev, "Power management not supported\n");
        return -ENODEV;
    }

    /* Initialize with balanced profile */
    profile.power_limit = GPU_POWER_LIMIT_DEFAULT;
    profile.fan_speed = 40;
    profile.dynamic_control = true;

    return anarchy_power_set_profile(adev, &profile);
}

void anarchy_power_exit(struct anarchy_device *adev)
{
    struct power_profile profile;

    if (!adev)
        return;

    /* Reset to default conservative profile */
    profile.power_limit = GPU_POWER_LIMIT_MIN;
    profile.fan_speed = 30;
    profile.dynamic_control = false;

    anarchy_power_set_profile(adev, &profile);
}

int anarchy_power_set_profile(struct anarchy_device *adev, struct power_profile *profile)
{
    int ret;

    if (!adev || !profile)
        return -EINVAL;

    ret = apply_power_profile(adev, profile);
    if (ret)
        return ret;

    adev->power_profile = *profile;
    return 0;
}

int anarchy_power_get_profile(struct anarchy_device *adev, struct power_profile *profile)
{
    u32 reg;

    if (!adev || !profile)
        return -EINVAL;

    /* Read current power limit */
    reg = readl(adev->mmio_base + PWR_STATUS_OFFSET);
    profile->power_limit = reg & 0xFFFF;

    /* Read fan control */
    reg = readl(adev->mmio_base + FAN_CTRL_OFFSET);
    profile->fan_speed = reg & 0x7F;
    profile->dynamic_control = !!(reg & BIT(31));

    return 0;
}

int anarchy_power_set_fan_speed(struct anarchy_device *adev, unsigned int speed)
{
    u32 reg;

    if (!adev || speed > 100)
        return -EINVAL;

    /* Update fan speed while preserving dynamic control bit */
    reg = readl(adev->mmio_base + FAN_CTRL_OFFSET);
    reg &= BIT(31);  /* Keep dynamic control bit */
    reg |= speed;
    writel(reg, adev->mmio_base + FAN_CTRL_OFFSET);

    adev->power_profile.fan_speed = speed;
    return 0;
}

int anarchy_power_set_power_limit(struct anarchy_device *adev, unsigned int limit)
{
    if (!adev || limit < GPU_POWER_LIMIT_MIN || limit > GPU_POWER_LIMIT_MAX)
        return -EINVAL;

    /* Update power limit */
    writel(limit, adev->mmio_base + PWR_LIMIT_OFFSET);
    writel(1, adev->mmio_base + PWR_CTRL_OFFSET); /* Apply changes */

    adev->power_profile.power_limit = limit;
    return 0;
}

EXPORT_SYMBOL_GPL(anarchy_power_init);
EXPORT_SYMBOL_GPL(anarchy_power_exit);
EXPORT_SYMBOL_GPL(anarchy_power_set_profile);
EXPORT_SYMBOL_GPL(anarchy_power_get_profile);
EXPORT_SYMBOL_GPL(anarchy_power_set_fan_speed);
EXPORT_SYMBOL_GPL(anarchy_power_set_power_limit);
