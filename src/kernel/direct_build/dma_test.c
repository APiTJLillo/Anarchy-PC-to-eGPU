#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/ktime.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"
#include "anarchy-test.h"

/**
 * test_dma_ring_alloc - Test DMA ring allocation and initialization
 */
static int test_dma_ring_alloc(struct anarchy_device *dev,
                             struct anarchy_test_result *result)
{
    struct anarchy_ring *tx_ring = &dev->tx_ring;
    struct anarchy_ring *rx_ring = &dev->rx_ring;
    int ret;

    /* Test TX ring allocation */
    ret = anarchy_ring_start(dev, tx_ring, ANARCHY_RING_TX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to allocate TX ring: %d", ret);
        return ret;
    }

    /* Verify TX ring properties */
    if (!tx_ring->desc || !tx_ring->dma_addr) {
        result->message = kasprintf(GFP_KERNEL, "TX ring descriptors not allocated");
        ret = -EINVAL;
        goto cleanup_tx;
    }

    /* Test RX ring allocation */
    ret = anarchy_ring_start(dev, rx_ring, ANARCHY_RING_RX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to allocate RX ring: %d", ret);
        goto cleanup_tx;
    }

    /* Verify RX ring properties */
    if (!rx_ring->desc || !rx_ring->dma_addr) {
        result->message = kasprintf(GFP_KERNEL, "RX ring descriptors not allocated");
        ret = -EINVAL;
        goto cleanup_rx;
    }

    /* Success */
    result->passed = true;

cleanup_rx:
    anarchy_ring_stop(dev, rx_ring);
cleanup_tx:
    anarchy_ring_stop(dev, tx_ring);
    return ret;
}

/**
 * test_dma_transfer - Test basic DMA data transfer
 */
static int test_dma_transfer(struct anarchy_device *dev,
                          struct anarchy_test_result *result)
{
    struct anarchy_ring *tx_ring = &dev->tx_ring;
    struct anarchy_ring *rx_ring = &dev->rx_ring;
    void *test_data;
    dma_addr_t test_dma;
    const size_t test_size = PAGE_SIZE;
    int ret;

    /* Allocate test buffer */
    test_data = dma_alloc_coherent(dev->dev, test_size, &test_dma, GFP_KERNEL);
    if (!test_data) {
        result->message = kasprintf(GFP_KERNEL, "Failed to allocate test buffer");
        return -ENOMEM;
    }

    /* Initialize rings */
    ret = anarchy_ring_start(dev, tx_ring, ANARCHY_RING_TX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start TX ring: %d", ret);
        goto free_buffer;
    }

    ret = anarchy_ring_start(dev, rx_ring, ANARCHY_RING_RX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start RX ring: %d", ret);
        goto stop_tx;
    }

    /* Queue transfer */
    ret = anarchy_ring_transfer(dev, test_dma, test_size);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to queue transfer: %d", ret);
        goto cleanup;
    }

    /* Wait for completion */
    ret = anarchy_ring_complete(dev);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Transfer completion failed: %d", ret);
        goto cleanup;
    }

    /* Success */
    result->passed = true;

cleanup:
    anarchy_ring_stop(dev, rx_ring);
stop_tx:
    anarchy_ring_stop(dev, tx_ring);
free_buffer:
    dma_free_coherent(dev->dev, test_size, test_data, test_dma);
    return ret;
}

/**
 * test_dma_error_handling - Test DMA error injection and recovery
 */
static int test_dma_error_handling(struct anarchy_device *dev,
                                struct anarchy_test_result *result)
{
    struct anarchy_ring *tx_ring = &dev->tx_ring;
    struct anarchy_ring *rx_ring = &dev->rx_ring;
    int ret;

    /* Enable error injection */
    anarchy_error_enable(ANARCHY_ERROR_DMA_TIMEOUT);

    /* Initialize rings */
    ret = anarchy_ring_start(dev, tx_ring, ANARCHY_RING_TX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start TX ring: %d", ret);
        goto disable_error;
    }

    ret = anarchy_ring_start(dev, rx_ring, ANARCHY_RING_RX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start RX ring: %d", ret);
        goto stop_tx;
    }

    /* Attempt transfer with error injection */
    ret = anarchy_ring_transfer(dev, 0, PAGE_SIZE);
    if (ret != -ETIMEDOUT) {
        result->message = kasprintf(GFP_KERNEL, 
                                  "Expected timeout error, got: %d", ret);
        goto cleanup;
    }

    /* Verify rings are reset after error */
    if (tx_ring->state != ANARCHY_RING_STATE_READY ||
        rx_ring->state != ANARCHY_RING_STATE_READY) {
        result->message = kasprintf(GFP_KERNEL, "Rings not properly reset after error");
        ret = -EINVAL;
        goto cleanup;
    }

    /* Success */
    result->passed = true;
    ret = 0;

cleanup:
    anarchy_ring_stop(dev, rx_ring);
stop_tx:
    anarchy_ring_stop(dev, tx_ring);
disable_error:
    anarchy_error_disable(ANARCHY_ERROR_DMA_TIMEOUT);
    return ret;
}

/**
 * test_dma_stress - Stress test DMA transfers
 */
static int test_dma_stress(struct anarchy_device *dev,
                         struct anarchy_test_result *result)
{
    struct anarchy_ring *tx_ring = &dev->tx_ring;
    struct anarchy_ring *rx_ring = &dev->rx_ring;
    void *test_data;
    dma_addr_t test_dma;
    const size_t test_size = PAGE_SIZE;
    int i, ret;

    /* Allocate test buffer */
    test_data = dma_alloc_coherent(dev->dev, test_size, &test_dma, GFP_KERNEL);
    if (!test_data) {
        result->message = kasprintf(GFP_KERNEL, "Failed to allocate test buffer");
        return -ENOMEM;
    }

    /* Initialize rings */
    ret = anarchy_ring_start(dev, tx_ring, ANARCHY_RING_TX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start TX ring: %d", ret);
        goto free_buffer;
    }

    ret = anarchy_ring_start(dev, rx_ring, ANARCHY_RING_RX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start RX ring: %d", ret);
        goto stop_tx;
    }

    /* Perform multiple transfers */
    for (i = 0; i < 1000; i++) {
        ret = anarchy_ring_transfer(dev, test_dma, test_size);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Transfer %d failed: %d", i, ret);
            goto cleanup;
        }

        ret = anarchy_ring_complete(dev);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Completion %d failed: %d", i, ret);
            goto cleanup;
        }

        /* Small delay between transfers */
        usleep_range(100, 200);
    }

    /* Success */
    result->passed = true;
    ret = 0;

cleanup:
    anarchy_ring_stop(dev, rx_ring);
stop_tx:
    anarchy_ring_stop(dev, tx_ring);
free_buffer:
    dma_free_coherent(dev->dev, test_size, test_data, test_dma);
    return ret;
}

/**
 * test_dma_performance - Measure DMA transfer performance
 */
static int test_dma_performance(struct anarchy_device *dev,
                             struct anarchy_test_result *result)
{
    struct anarchy_ring *tx_ring = &dev->tx_ring;
    struct anarchy_ring *rx_ring = &dev->rx_ring;
    void *test_data;
    dma_addr_t test_dma;
    const size_t test_size = PAGE_SIZE * 256; /* 1MB transfer size */
    const int iterations = 100;
    ktime_t start, end;
    s64 total_time_ns = 0;
    u64 total_bytes = 0;
    int i, ret;

    /* Allocate large test buffer */
    test_data = dma_alloc_coherent(dev->dev, test_size, &test_dma, GFP_KERNEL);
    if (!test_data) {
        result->message = kasprintf(GFP_KERNEL, "Failed to allocate test buffer");
        return -ENOMEM;
    }

    /* Initialize rings */
    ret = anarchy_ring_start(dev, tx_ring, ANARCHY_RING_TX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start TX ring: %d", ret);
        goto free_buffer;
    }

    ret = anarchy_ring_start(dev, rx_ring, ANARCHY_RING_RX);
    if (ret) {
        result->message = kasprintf(GFP_KERNEL, "Failed to start RX ring: %d", ret);
        goto stop_tx;
    }

    /* Perform timed transfers */
    for (i = 0; i < iterations; i++) {
        start = ktime_get();

        ret = anarchy_ring_transfer(dev, test_dma, test_size);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Transfer %d failed: %d", i, ret);
            goto cleanup;
        }

        ret = anarchy_ring_complete(dev);
        if (ret) {
            result->message = kasprintf(GFP_KERNEL,
                                      "Completion %d failed: %d", i, ret);
            goto cleanup;
        }

        end = ktime_get();
        total_time_ns += ktime_to_ns(ktime_sub(end, start));
        total_bytes += test_size;
    }

    /* Calculate and report performance metrics */
    {
        u64 avg_latency_us = div_u64(total_time_ns, iterations * 1000);
        u64 throughput_mbps = div_u64(total_bytes * 8000, total_time_ns);

        result->message = kasprintf(GFP_KERNEL,
                                  "DMA Performance: Throughput=%llu Mbps, Latency=%llu us",
                                  throughput_mbps, avg_latency_us);
        result->passed = true;
        ret = 0;
    }

cleanup:
    anarchy_ring_stop(dev, rx_ring);
stop_tx:
    anarchy_ring_stop(dev, tx_ring);
free_buffer:
    dma_free_coherent(dev->dev, test_size, test_data, test_dma);
    return ret;
}

/* DMA test cases */
struct anarchy_test_case anarchy_dma_tests[] = {
    ANARCHY_TEST_CASE(dma_ring_alloc, ANARCHY_TEST_DMA,
                     test_dma_ring_alloc),
    ANARCHY_TEST_CASE(dma_transfer, ANARCHY_TEST_DMA,
                     test_dma_transfer),
    ANARCHY_TEST_CASE(dma_error_handling, ANARCHY_TEST_DMA,
                     test_dma_error_handling),
    ANARCHY_TEST_CASE(dma_stress, ANARCHY_TEST_STRESS,
                     test_dma_stress),
    ANARCHY_TEST_CASE(dma_performance, ANARCHY_TEST_PERFORMANCE,
                     test_dma_performance),
    { NULL }  /* Sentinel */
}; 