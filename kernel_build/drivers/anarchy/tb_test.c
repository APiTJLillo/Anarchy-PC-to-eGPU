#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-test.h"

/**
 * test_tb_service_registration - Test Thunderbolt service registration
 */
static int test_tb_service_registration(struct anarchy_device *dev,
                                      struct anarchy_test_result *result)
{
    int ret;
    
    /* Try to register service with error injection */
    if (anarchy_should_inject_error(ANARCHY_ERR_TB_CONNECT)) {
        result->message = kasprintf(GFP_KERNEL, "Injected connection failure");
        return -ECONNREFUSED;
    }
    
    /* Verify service properties */
    if (!dev->xd) {
        result->message = kasprintf(GFP_KERNEL, "XDomain connection not established");
        return -ENOTCONN;
    }
    
    result->passed = true;
    return 0;
}

/**
 * test_tb_ring_setup - Test DMA ring setup
 */
static int test_tb_ring_setup(struct anarchy_device *dev,
                            struct anarchy_test_result *result)
{
    int ret;
    
    /* Test TX ring setup */
    ret = anarchy_ring_start(dev, &dev->tx_ring, true);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "TX ring setup failed: %d", ret);
        return ret;
    }
    
    /* Test RX ring setup */
    ret = anarchy_ring_start(dev, &dev->rx_ring, false);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "RX ring setup failed: %d", ret);
        anarchy_ring_stop(dev, &dev->tx_ring);
        return ret;
    }
    
    /* Clean up */
    anarchy_ring_stop(dev, &dev->rx_ring);
    anarchy_ring_stop(dev, &dev->tx_ring);
    
    result->passed = true;
    return 0;
}

/**
 * test_tb_data_transfer - Test basic data transfer
 */
static int test_tb_data_transfer(struct anarchy_device *dev,
                               struct anarchy_test_result *result)
{
    struct anarchy_transfer transfer = {0};
    char test_data[] = "Anarchy eGPU Test Data";
    char rx_buffer[64] = {0};
    int ret;
    
    /* Set up transfer */
    transfer.data = test_data;
    transfer.size = sizeof(test_data);
    transfer.flags = ANARCHY_TRANSFER_SYNC;
    
    /* Start rings */
    ret = anarchy_ring_start(dev, &dev->tx_ring, true);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "TX ring setup failed: %d", ret);
        return ret;
    }
    
    ret = anarchy_ring_start(dev, &dev->rx_ring, false);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "RX ring setup failed: %d", ret);
        goto err_stop_tx;
    }
    
    /* Perform transfer */
    ret = anarchy_ring_transfer(dev, &dev->tx_ring, &transfer);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Transfer failed: %d", ret);
        goto err_stop_rx;
    }
    
    /* Verify transfer completion */
    if (transfer.status != 0) {
        result->message = kasprintf(GFP_KERNEL, "Transfer status error: %d",
                                  transfer.status);
        ret = -EIO;
        goto err_stop_rx;
    }
    
    result->passed = true;
    
err_stop_rx:
    anarchy_ring_stop(dev, &dev->rx_ring);
err_stop_tx:
    anarchy_ring_stop(dev, &dev->tx_ring);
    return ret;
}

/**
 * test_tb_stress - Stress test Thunderbolt connection
 */
static int test_tb_stress(struct anarchy_device *dev,
                         struct anarchy_test_result *result)
{
    struct anarchy_transfer transfer = {0};
    char test_data[4096];
    int i, ret;
    
    /* Initialize test data */
    for (i = 0; i < sizeof(test_data); i++)
        test_data[i] = i & 0xff;
    
    /* Set up transfer */
    transfer.data = test_data;
    transfer.size = sizeof(test_data);
    transfer.flags = ANARCHY_TRANSFER_SYNC;
    
    /* Start rings */
    ret = anarchy_ring_start(dev, &dev->tx_ring, true);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "TX ring setup failed: %d", ret);
        return ret;
    }
    
    ret = anarchy_ring_start(dev, &dev->rx_ring, false);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "RX ring setup failed: %d", ret);
        goto err_stop_tx;
    }
    
    /* Perform multiple transfers */
    for (i = 0; i < 1000; i++) {
        ret = anarchy_ring_transfer(dev, &dev->tx_ring, &transfer);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL, 
                                      "Transfer %d failed: %d", i, ret);
            goto err_stop_rx;
        }
        
        if (transfer.status != 0) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Transfer %d status error: %d",
                                      i, transfer.status);
            ret = -EIO;
            goto err_stop_rx;
        }
        
        /* Small delay between transfers */
        usleep_range(1000, 2000);
    }
    
    result->passed = true;
    
err_stop_rx:
    anarchy_ring_stop(dev, &dev->rx_ring);
err_stop_tx:
    anarchy_ring_stop(dev, &dev->tx_ring);
    return ret;
}

/* Thunderbolt test cases */
struct anarchy_test_case anarchy_tb_tests[] = {
    ANARCHY_TEST_CASE(tb_service_registration, ANARCHY_TEST_TB,
                     test_tb_service_registration),
    ANARCHY_TEST_CASE(tb_ring_setup, ANARCHY_TEST_TB,
                     test_tb_ring_setup),
    ANARCHY_TEST_CASE(tb_data_transfer, ANARCHY_TEST_TB,
                     test_tb_data_transfer),
    ANARCHY_TEST_CASE(tb_stress, ANARCHY_TEST_STRESS,
                     test_tb_stress),
    { NULL }  /* Sentinel */
}; 