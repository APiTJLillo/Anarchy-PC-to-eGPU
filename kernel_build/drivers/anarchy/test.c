#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/slab.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-test.h"

/* Global test configuration */
struct anarchy_error_config anarchy_errors = {
    .error_mask = 0,
    .error_frequency = 100,  /* 1% error rate by default */
    .enabled = false,
};

struct anarchy_test_stats anarchy_stats = {
    .total_tests = ATOMIC_INIT(0),
    .passed_tests = ATOMIC_INIT(0),
    .failed_tests = ATOMIC_INIT(0),
    .skipped_tests = ATOMIC_INIT(0),
    .total_duration = ATOMIC64_INIT(0),
};

/* Test statistics */
static struct {
    unsigned int total;
    unsigned int passed;
    unsigned int failed;
} test_stats;

/**
 * anarchy_error_init - Initialize error injection system
 */
void anarchy_error_init(void)
{
    anarchy_errors.error_mask = 0;
    anarchy_errors.error_frequency = 100;
    anarchy_errors.enabled = false;
    
    anarchy_info("Error injection system initialized\n");
}

/**
 * anarchy_error_enable - Enable specific error injections
 * @error_mask: Mask of errors to enable
 */
void anarchy_error_enable(u32 error_mask)
{
    anarchy_errors.error_mask |= error_mask;
    anarchy_errors.enabled = true;
    
    anarchy_info("Error injection enabled (mask=0x%x)\n", error_mask);
}

/**
 * anarchy_error_disable - Disable specific error injections
 * @error_mask: Mask of errors to disable
 */
void anarchy_error_disable(u32 error_mask)
{
    anarchy_errors.error_mask &= ~error_mask;
    if (!anarchy_errors.error_mask)
        anarchy_errors.enabled = false;
        
    anarchy_info("Error injection disabled (mask=0x%x)\n", error_mask);
}

/**
 * anarchy_should_inject_error - Check if an error should be injected
 * @error_type: Type of error to check
 *
 * Returns true if an error should be injected.
 */
bool anarchy_should_inject_error(u32 error_type)
{
    u32 rand;
    
    if (!anarchy_errors.enabled || !(anarchy_errors.error_mask & error_type))
        return false;
        
    get_random_bytes(&rand, sizeof(rand));
    return (rand % anarchy_errors.error_frequency) == 0;
}

/**
 * run_test_suite - Run all tests in a test suite
 */
static int run_test_suite(struct anarchy_device *dev,
                         struct anarchy_test_case *suite,
                         unsigned int categories)
{
    struct anarchy_test_result result;
    struct anarchy_test_case *test;
    int ret = 0;

    for (test = suite; test->name != NULL; test++) {
        /* Skip if test category doesn't match */
        if (!(test->categories & categories))
            continue;

        /* Initialize result */
        memset(&result, 0, sizeof(result));

        pr_info("Running test: %s\n", test->name);

        /* Run test */
        ret = test->run(dev, &result);
        test_stats.total++;

        if (result.passed) {
            test_stats.passed++;
            pr_info("Test %s: PASSED\n", test->name);
            if (result.message)
                pr_info("%s\n", result.message);
        } else {
            test_stats.failed++;
            pr_err("Test %s: FAILED - %s (ret=%d)\n",
                  test->name, result.message ? result.message : "No details", ret);
        }

        /* Free result message if allocated */
        kfree(result.message);

        if (ret && !(categories & ANARCHY_TEST_STRESS))
            break;
    }

    return ret;
}

/**
 * anarchy_run_tests - Run all tests matching the specified categories
 */
int anarchy_run_tests(struct anarchy_device *dev, unsigned int categories)
{
    int ret;

    /* Reset statistics */
    memset(&test_stats, 0, sizeof(test_stats));

    pr_info("Starting test run (categories=0x%x)\n", categories);

    /* Run Thunderbolt tests first */
    if (categories & ANARCHY_TEST_THUNDERBOLT) {
        ret = run_test_suite(dev, anarchy_tb_tests, categories);
        if (ret)
            goto out;
    }

    /* Run DMA tests */
    if (categories & ANARCHY_TEST_DMA) {
        ret = run_test_suite(dev, anarchy_dma_tests, categories);
        if (ret)
            goto out;
    }

    /* Run PCIe tests */
    if (categories & ANARCHY_TEST_PCIE) {
        ret = run_test_suite(dev, anarchy_pcie_tests, categories);
        if (ret)
            goto out;
    }

    /* Run stress tests last */
    if (categories & ANARCHY_TEST_STRESS) {
        pr_info("Starting stress tests (this may take a while)...\n");
        ret = run_test_suite(dev, anarchy_tb_tests, ANARCHY_TEST_STRESS);
        if (ret)
            goto out;
            
        ret = run_test_suite(dev, anarchy_dma_tests, ANARCHY_TEST_STRESS);
        if (ret)
            goto out;
            
        ret = run_test_suite(dev, anarchy_pcie_tests, ANARCHY_TEST_STRESS);
    }

out:
    pr_info("Test run complete: %u total, %u passed, %u failed\n",
            test_stats.total, test_stats.passed, test_stats.failed);
    return ret;
}

/**
 * anarchy_print_test_results - Print test run statistics
 */
void anarchy_print_test_results(void)
{
    pr_info("Test Results:\n");
    pr_info("  Total tests:  %u\n", test_stats.total);
    pr_info("  Passed:       %u\n", test_stats.passed);
    pr_info("  Failed:       %u\n", test_stats.failed);
    pr_info("  Success rate: %u%%\n",
            test_stats.total ? (test_stats.passed * 100) / test_stats.total : 0);
}

/**
 * anarchy_test_init - Initialize test framework
 */
int anarchy_test_init(void)
{
    anarchy_error_init();
    anarchy_info("Test framework initialized\n");
    return 0;
}

/**
 * anarchy_test_exit - Clean up test framework
 */
void anarchy_test_exit(void)
{
    anarchy_info("Test framework cleanup complete\n");
}

/* Module parameters for error injection */
module_param_named(error_mask, anarchy_errors.error_mask, uint, 0644);
MODULE_PARM_DESC(error_mask, "Error injection mask (bitfield)");

module_param_named(error_frequency, anarchy_errors.error_frequency, uint, 0644);
MODULE_PARM_DESC(error_frequency, "Error injection frequency (1 in N)");

module_param_named(error_enabled, anarchy_errors.enabled, bool, 0644);
MODULE_PARM_DESC(error_enabled, "Enable error injection (0/1)"); 