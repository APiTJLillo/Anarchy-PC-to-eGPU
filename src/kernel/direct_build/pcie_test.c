#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-test.h"

/* PCI class for VGA controllers */
#define PCI_CLASS_VGA_BASE 0x030000

/* Test device instance */
static struct anarchy_device *test_dev;

/* Test helper functions */
static void dump_config_space(struct anarchy_device *adev, const char *msg)
{
    u32 val;
    int i;
    
    pr_info("PCIe Config Space Dump: %s\n", msg);
    for (i = 0; i < 0x40; i += 4) {
        anarchy_pcie_read_config(adev, i, 4, &val);
        pr_info("  0x%02x: 0x%08x\n", i, val);
    }
}

/* Test cases */

static int test_config_header(struct anarchy_device *adev)
{
    u32 val;
    int ret = 0;

    pr_info("Testing PCIe Configuration Header...\n");

    /* Test vendor/device ID */
    anarchy_pcie_read_config(adev, PCI_VENDOR_ID, 4, &val);
    if ((val & 0xffff) != 0x10DE) {  /* NVIDIA vendor ID */
        pr_err("  Wrong vendor ID: got 0x%04x, expected 0x%04x\n",
               val & 0xffff, 0x10DE);
        ret = -EINVAL;
    }
    if (((val >> 16) & 0xffff) != 0x2684) {  /* RTX 4090 device ID */
        pr_err("  Wrong device ID: got 0x%04x, expected 0x%04x\n",
               (val >> 16) & 0xffff, 0x2684);
        ret = -EINVAL;
    }

    /* Test class code */
    anarchy_pcie_read_config(adev, PCI_CLASS_REVISION, 4, &val);
    if ((val >> 8) != PCI_CLASS_VGA_BASE) {
        pr_err("  Wrong class code: got 0x%06x, expected 0x%06x\n",
               val >> 8, PCI_CLASS_VGA_BASE);
        ret = -EINVAL;
    }

    if (!ret)
        pr_info("  Configuration header test passed\n");
    return ret;
}

static int test_bar_configuration(struct anarchy_device *adev)
{
    u32 val;
    int ret = 0;

    pr_info("Testing BAR Configuration...\n");

    /* Test BAR0 (MMIO) */
    anarchy_pcie_read_config(adev, PCI_BASE_ADDRESS_0, 4, &val);
    if ((val & ~0xf) != (adev->pcie_cfg.bar[0] & ~0xf)) {
        pr_err("  BAR0 mismatch: got 0x%08x, expected 0x%08x\n",
               val & ~0xf, (u32)adev->pcie_cfg.bar[0] & ~0xf);
        ret = -EINVAL;
    }

    /* Test BAR2/3 (Framebuffer) */
    anarchy_pcie_read_config(adev, PCI_BASE_ADDRESS_2, 4, &val);
    if ((val & ~0xf) != (adev->pcie_cfg.bar[2] & ~0xf)) {
        pr_err("  BAR2 mismatch: got 0x%08x, expected 0x%08x\n",
               val & ~0xf, (u32)adev->pcie_cfg.bar[2] & ~0xf);
        ret = -EINVAL;
    }

    if (!ret)
        pr_info("  BAR configuration test passed\n");
    return ret;
}

static int test_capabilities(struct anarchy_device *adev)
{
    u32 val;
    int ret = 0;

    pr_info("Testing PCIe Capabilities...\n");

    /* Test capability pointer */
    anarchy_pcie_read_config(adev, PCI_CAPABILITY_LIST, 1, &val);
    if (val != adev->pcie_cfg.caps.pm_cap) {
        pr_err("  Wrong capability pointer: got 0x%02x, expected 0x%02x\n",
               val, adev->pcie_cfg.caps.pm_cap);
        ret = -EINVAL;
    }

    /* Test MSI capability */
    anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.msi_cap, 2, &val);
    if ((val & 0xff) != PCI_CAP_ID_MSI) {
        pr_err("  Wrong MSI capability ID: got 0x%02x, expected 0x%02x\n",
               val & 0xff, PCI_CAP_ID_MSI);
        ret = -EINVAL;
    }

    /* Test PCIe capability */
    anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.pcie_cap, 2, &val);
    if ((val & 0xff) != PCI_CAP_ID_EXP) {
        pr_err("  Wrong PCIe capability ID: got 0x%02x, expected 0x%02x\n",
               val & 0xff, PCI_CAP_ID_EXP);
        ret = -EINVAL;
    }

    if (!ret)
        pr_info("  Capabilities test passed\n");
    return ret;
}

static int test_link_training(struct anarchy_device *adev)
{
    u32 val;
    int ret = 0;

    pr_info("Testing Link Training...\n");

    /* Read initial link status */
    anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.pcie_cap + 0x12, 2, &val);
    pr_info("  Initial link status: 0x%04x\n", val);

    /* Trigger link retrain */
    anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.pcie_cap + 0x10, 2, &val);
    val |= PCI_EXP_LNKCTL_RL;
    anarchy_pcie_write_config(adev, adev->pcie_cfg.caps.pcie_cap + 0x10, 2, val);

    /* Wait for retraining */
    msleep(200);

    /* Verify link status */
    anarchy_pcie_read_config(adev, adev->pcie_cfg.caps.pcie_cap + 0x12, 2, &val);
    if (((val >> 4) & 0x3f) != adev->max_lanes) {
        pr_err("  Wrong link width: got x%d, expected x%d\n",
               (val >> 4) & 0x3f, adev->max_lanes);
        ret = -EINVAL;
    }
    if ((val & 0xf) != adev->max_speed) {
        pr_err("  Wrong link speed: got Gen%d, expected Gen%d\n",
               val & 0xf, adev->max_speed);
        ret = -EINVAL;
    }

    if (!ret)
        pr_info("  Link training test passed\n");
    return ret;
}

static int test_memory_regions(struct anarchy_device *adev)
{
    int ret = 0;

    pr_info("Testing Memory Regions...\n");

    /* Verify MMIO region */
    if (!adev->pcie_cfg.mmio_base) {
        pr_err("  MMIO region not mapped\n");
        ret = -ENOMEM;
    }

    /* Verify framebuffer region */
    if (!adev->pcie_cfg.fb_base) {
        pr_err("  Framebuffer region not mapped\n");
        ret = -ENOMEM;
    }

    if (!ret)
        pr_info("  Memory regions test passed\n");
    return ret;
}

/**
 * run_pcie_tests - Run all PCIe emulation tests
 */
static int run_pcie_tests(void)
{
    int ret;

    /* Allocate test device */
    test_dev = kzalloc(sizeof(*test_dev), GFP_KERNEL);
    if (!test_dev)
        return -ENOMEM;

    /* Initialize test device */
    test_dev->max_lanes = ANARCHY_MAX_LANES;
    test_dev->max_speed = ANARCHY_PCIE_GEN4;

    /* Initialize PCIe emulation */
    ret = anarchy_pcie_init(test_dev);
    if (ret) {
        pr_err("Failed to initialize PCIe emulation: %d\n", ret);
        goto err_free;
    }

    /* Dump initial config space */
    dump_config_space(test_dev, "Initial state");

    /* Run test cases */
    ret = test_config_header(test_dev);
    if (ret)
        goto err_exit;

    ret = test_bar_configuration(test_dev);
    if (ret)
        goto err_exit;

    ret = test_capabilities(test_dev);
    if (ret)
        goto err_exit;

    ret = test_link_training(test_dev);
    if (ret)
        goto err_exit;

    ret = test_memory_regions(test_dev);
    if (ret)
        goto err_exit;

    pr_info("All PCIe emulation tests passed!\n");
    anarchy_pcie_exit(test_dev);
    kfree(test_dev);
    return 0;

err_exit:
    anarchy_pcie_exit(test_dev);
err_free:
    kfree(test_dev);
    return ret;
}

/* Module initialization */
static int __init pcie_test_init(void)
{
    pr_info("Starting PCIe emulation tests...\n");
    return run_pcie_tests();
}

/* Module cleanup */
static void __exit pcie_test_exit(void)
{
    pr_info("PCIe emulation tests complete\n");
}

module_init(pcie_test_init);
module_exit(pcie_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("PCIe Emulation Test Suite"); 