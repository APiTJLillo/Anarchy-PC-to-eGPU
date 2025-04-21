#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/uuid.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-test.h"

/**
 * test_thunderbolt_service - Test Thunderbolt service registration
 */
static int test_thunderbolt_service(struct anarchy_device *dev,
                                 struct anarchy_test_result *result)
{
    int ret;

    /* Test service registration */
    ret = anarchy_tb_register_service(dev);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to register service: %d", ret);
        return ret;
    }

    /* Verify service UUID */
    if (!uuid_equal(&dev->service_uuid, &ANARCHY_SERVICE_UUID)) {
        result->message = kasprintf(GFP_KERNEL, "Service UUID mismatch");
        ret = -EINVAL;
        goto cleanup;
    }

    /* Verify property directory */
    if (!dev->properties_dir) {
        result->message = kasprintf(GFP_KERNEL, "Properties directory not created");
        ret = -EINVAL;
        goto cleanup;
    }

    /* Success */
    result->passed = true;
    ret = 0;

cleanup:
    anarchy_tb_unregister_service(dev);
    return ret;
}

/**
 * test_thunderbolt_connection - Test Thunderbolt connection establishment
 */
static int test_thunderbolt_connection(struct anarchy_device *dev,
                                    struct anarchy_test_result *result)
{
    int ret;

    /* Initialize connection */
    ret = anarchy_tb_init(dev);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to initialize connection: %d", ret);
        return ret;
    }

    /* Verify connection state */
    if (dev->tb_state != ANARCHY_TB_STATE_READY) {
        result->message = kasprintf(GFP_KERNEL, 
                                  "Invalid connection state: %d", dev->tb_state);
        ret = -EINVAL;
        goto cleanup;
    }

    /* Test basic properties */
    if (!dev->xdomain || !dev->service) {
        result->message = kasprintf(GFP_KERNEL, "Connection resources not allocated");
        ret = -EINVAL;
        goto cleanup;
    }

    /* Success */
    result->passed = true;
    ret = 0;

cleanup:
    anarchy_tb_exit(dev);
    return ret;
}

/**
 * test_thunderbolt_error_handling - Test Thunderbolt error handling
 */
static int test_thunderbolt_error_handling(struct anarchy_device *dev,
                                        struct anarchy_test_result *result)
{
    int ret;

    /* Enable connection error injection */
    anarchy_error_enable(ANARCHY_ERR_TB_CONNECT);

    /* Attempt connection with error injection */
    ret = anarchy_tb_init(dev);
    if (ret != -ECONNREFUSED) {
        result->message = kasprintf(GFP_KERNEL,
                                  "Expected connection refusal, got: %d", ret);
        goto disable_error;
    }

    /* Verify cleanup after error */
    if (dev->tb_state != ANARCHY_TB_STATE_INIT) {
        result->message = kasprintf(GFP_KERNEL,
                                  "Invalid state after error: %d", dev->tb_state);
        ret = -EINVAL;
        goto disable_error;
    }

    /* Disable error injection and retry */
    anarchy_error_disable(ANARCHY_ERR_TB_CONNECT);
    ret = anarchy_tb_init(dev);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL,
                                  "Failed to recover after error: %d", ret);
        goto cleanup;
    }

    /* Success */
    result->passed = true;
    ret = 0;

cleanup:
    anarchy_tb_exit(dev);
    return ret;

disable_error:
    anarchy_error_disable(ANARCHY_ERR_TB_CONNECT);
    return ret;
}

/**
 * test_thunderbolt_stress - Stress test Thunderbolt connection
 */
static int test_thunderbolt_stress(struct anarchy_device *dev,
                                struct anarchy_test_result *result)
{
    int i, ret;

    for (i = 0; i < 100; i++) {
        /* Initialize connection */
        ret = anarchy_tb_init(dev);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Connection %d failed: %d", i, ret);
            return ret;
        }

        /* Verify connection state */
        if (dev->tb_state != ANARCHY_TB_STATE_READY) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Invalid state in iteration %d: %d",
                                      i, dev->tb_state);
            ret = -EINVAL;
            goto cleanup;
        }

        /* Cleanup */
        anarchy_tb_exit(dev);

        /* Small delay between iterations */
        msleep(100);
    }

    /* Success */
    result->passed = true;
    return 0;

cleanup:
    anarchy_tb_exit(dev);
    return ret;
}

/**
 * test_thunderbolt_performance - Test Thunderbolt connection performance
 */
static int test_thunderbolt_performance(struct anarchy_device *dev,
                                     struct anarchy_test_result *result)
{
    ktime_t start, end;
    s64 total_time_ns = 0;
    int i, ret;

    /* Time 100 connection cycles */
    for (i = 0; i < 100; i++) {
        start = ktime_get();

        ret = anarchy_tb_init(dev);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Connection %d failed: %d", i, ret);
            return ret;
        }

        anarchy_tb_exit(dev);

        end = ktime_get();
        total_time_ns += ktime_to_ns(ktime_sub(end, start));
    }

    /* Calculate and report metrics */
    {
        u64 avg_time_us = div_u64(total_time_ns, 100 * 1000);
        result->message = kasprintf(GFP_KERNEL,
                                  "Average connection time: %llu us",
                                  avg_time_us);
        result->passed = true;
    }

    return 0;
}

/* Thunderbolt test cases */
struct anarchy_test_case anarchy_tb_tests[] = {
    ANARCHY_TEST_CASE(thunderbolt_service, ANARCHY_TEST_THUNDERBOLT,
                     test_thunderbolt_service),
    ANARCHY_TEST_CASE(thunderbolt_connection, ANARCHY_TEST_THUNDERBOLT,
                     test_thunderbolt_connection),
    ANARCHY_TEST_CASE(thunderbolt_error_handling, ANARCHY_TEST_THUNDERBOLT,
                     test_thunderbolt_error_handling),
    ANARCHY_TEST_CASE(thunderbolt_stress, ANARCHY_TEST_STRESS,
                     test_thunderbolt_stress),
    ANARCHY_TEST_CASE(thunderbolt_performance, ANARCHY_TEST_PERFORMANCE,
                     test_thunderbolt_performance),
    { NULL }  /* Sentinel */
}; 