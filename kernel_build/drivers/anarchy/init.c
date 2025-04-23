#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/debugfs.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-test.h"

/* Module parameters */
static bool enable_testing = false;
static unsigned int test_categories = 0;
static bool performance_mode = false;

/* Debugfs root directory */
static struct dentry *anarchy_debugfs_root;

/**
 * anarchy_egpu_probe - Initialize device on probe
 */
static int anarchy_egpu_probe(struct platform_device *pdev)
{
    struct anarchy_device *dev;
    int ret;

    /* Allocate device structure */
    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->dev = &pdev->dev;
    platform_set_drvdata(pdev, dev);

    /* Enable DMA */
    ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
    if (ret) {
        dev_err(&pdev->dev, "Failed to set DMA mask: %d\n", ret);
        return ret;
    }

    /* Create debugfs entries */
    if (anarchy_debugfs_root) {
        dev->debugfs_dir = debugfs_create_dir(dev_name(&pdev->dev),
                                            anarchy_debugfs_root);
        if (!dev->debugfs_dir)
            dev_warn(&pdev->dev, "Failed to create debugfs directory\n");
        else {
            debugfs_create_bool("performance_mode", 0644, dev->debugfs_dir,
                              &performance_mode);
            debugfs_create_u32("error_mask", 0644, dev->debugfs_dir,
                             &anarchy_errors.error_mask);
        }
    }

    /* Initialize Thunderbolt connection */
    ret = anarchy_tb_init(dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to initialize Thunderbolt: %d\n", ret);
        goto cleanup_debugfs;
    }

    /* Initialize PCIe emulation */
    ret = anarchy_pcie_init(dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to initialize PCIe: %d\n", ret);
        goto cleanup_thunderbolt;
    }

    /* Run tests if enabled */
    if (enable_testing && test_categories) {
        ret = anarchy_run_tests(dev, test_categories);
        if (ret) {
            dev_err(&pdev->dev, "Tests failed: %d\n", ret);
            goto cleanup_pcie;
        }
        anarchy_print_test_results();
    }

    dev_info(&pdev->dev, "Anarchy eGPU device initialized\n");
    return 0;

cleanup_pcie:
    anarchy_pcie_exit(dev);
cleanup_thunderbolt:
    anarchy_tb_exit(dev);
cleanup_debugfs:
    debugfs_remove_recursive(dev->debugfs_dir);
    return ret;
}

/**
 * anarchy_egpu_remove - Clean up device on removal
 */
static int anarchy_egpu_remove(struct platform_device *pdev)
{
    struct anarchy_device *dev = platform_get_drvdata(pdev);

    anarchy_pcie_exit(dev);
    anarchy_tb_exit(dev);
    debugfs_remove_recursive(dev->debugfs_dir);

    dev_info(&pdev->dev, "Anarchy eGPU device removed\n");
    return 0;
}

/* Platform driver structure */
static struct platform_driver anarchy_egpu_driver = {
    .probe = anarchy_egpu_probe,
    .remove = anarchy_egpu_remove,
    .driver = {
        .name = "anarchy-egpu",
        .owner = THIS_MODULE,
    },
};

/**
 * anarchy_init - Module initialization
 */
static int __init anarchy_init(void)
{
    int ret;

    /* Create debugfs root */
    anarchy_debugfs_root = debugfs_create_dir("anarchy-egpu", NULL);
    if (!anarchy_debugfs_root)
        pr_warn("Failed to create debugfs root directory\n");

    /* Initialize test framework */
    ret = anarchy_test_init();
    if (ret) {
        pr_err("Failed to initialize test framework: %d\n", ret);
        goto cleanup_debugfs;
    }

    /* Register platform driver */
    ret = platform_driver_register(&anarchy_egpu_driver);
    if (ret) {
        pr_err("Failed to register platform driver: %d\n", ret);
        goto cleanup_test;
    }

    pr_info("Anarchy eGPU driver loaded\n");
    return 0;

cleanup_test:
    anarchy_test_exit();
cleanup_debugfs:
    debugfs_remove_recursive(anarchy_debugfs_root);
    return ret;
}

/**
 * anarchy_exit - Module cleanup
 */
static void __exit anarchy_exit(void)
{
    platform_driver_unregister(&anarchy_egpu_driver);
    anarchy_test_exit();
    debugfs_remove_recursive(anarchy_debugfs_root);
    pr_info("Anarchy eGPU driver unloaded\n");
}

/* Module parameters */
module_param(enable_testing, bool, 0644);
MODULE_PARM_DESC(enable_testing, "Enable test execution on probe");

module_param(test_categories, uint, 0644);
MODULE_PARM_DESC(test_categories, "Test categories to run (bitfield)");

module_param(performance_mode, bool, 0644);
MODULE_PARM_DESC(performance_mode, "Enable performance optimizations");

module_init(anarchy_init);
module_exit(anarchy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("GPU passthrough over Thunderbolt");
MODULE_VERSION("1.0"); 