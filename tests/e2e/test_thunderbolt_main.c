#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/thunderbolt.h>
#include "../../include/anarchy-egpu.h"

/* Test data size */
#define TEST_DATA_SIZE (ANARCHY_RING_SIZE - sizeof(struct ring_frame))
#define TEST_ITERATIONS 100
#define TEST_TIMEOUT_MS 1000

static struct completion test_completion;
static int test_status;
static u8 *test_data;
static u8 *recv_data;

/* Get device function */
extern struct anarchy_device *anarchy_get_device(void);

static void test_transfer_complete(struct anarchy_device *adev,
                                 struct anarchy_ring *ring,
                                 struct anarchy_transfer *transfer,
                                 int status)
{
    if (status > 0) {
        memcpy(recv_data, transfer->data, status);
    }
    test_status = status;
    complete(&test_completion);
}

static int test_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    struct anarchy_transfer transfer = {0};
    int ret;
    unsigned long timeout;

    /* Initialize test data with a pattern */
    memset(test_data, 0xA5, TEST_DATA_SIZE);

    /* Setup transfer */
    transfer.data = test_data;
    transfer.size = TEST_DATA_SIZE;
    transfer.completed = false;

    /* Reset completion and status */
    reinit_completion(&test_completion);
    test_status = 0;

    /* Start transfer */
    ret = anarchy_ring_transfer(adev, ring, &transfer);
    if (ret) {
        pr_err("Failed to start transfer: %d\n", ret);
        return ret;
    }

    /* Wait for completion */
    timeout = wait_for_completion_timeout(&test_completion,
                                        msecs_to_jiffies(TEST_TIMEOUT_MS));
    if (!timeout) {
        pr_err("Transfer timeout\n");
        return -ETIMEDOUT;
    }

    if (test_status < 0) {
        pr_err("Transfer failed with status: %d\n", test_status);
        return test_status;
    }

    /* For RX ring, verify received data */
    if (ring == &adev->rx_ring && memcmp(test_data, recv_data, TEST_DATA_SIZE)) {
        pr_err("Data verification failed\n");
        return -EINVAL;
    }

    return 0;
}

static int test_ring_stress(struct anarchy_device *adev)
{
    int i, ret;

    for (i = 0; i < TEST_ITERATIONS; i++) {
        /* Test TX ring */
        ret = test_ring_transfer(adev, &adev->tx_ring);
        if (ret) {
            pr_err("TX ring test failed at iteration %d: %d\n", i, ret);
            return ret;
        }

        /* Small delay between transfers */
        usleep_range(1000, 2000);

        /* Test RX ring */
        ret = test_ring_transfer(adev, &adev->rx_ring);
        if (ret) {
            pr_err("RX ring test failed at iteration %d: %d\n", i, ret);
            return ret;
        }

        if ((i + 1) % 10 == 0)
            pr_info("Completed %d/%d iterations\n", i + 1, TEST_ITERATIONS);
    }

    return 0;
}

static int __init test_init(void)
{
    struct anarchy_device *adev;
    int ret;

    pr_info("Starting Anarchy eGPU e2e tests\n");

    /* Allocate test buffers */
    test_data = kmalloc(TEST_DATA_SIZE, GFP_KERNEL);
    recv_data = kmalloc(TEST_DATA_SIZE, GFP_KERNEL);
    if (!test_data || !recv_data) {
        pr_err("Failed to allocate test buffers\n");
        ret = -ENOMEM;
        goto err_free;
    }

    /* Initialize completion */
    init_completion(&test_completion);

    /* Initialize Thunderbolt */
    ret = anarchy_tb_init();
    if (ret) {
        pr_err("Failed to initialize Thunderbolt: %d\n", ret);
        goto err_free;
    }

    /* Wait for device connection */
    msleep(2000);

    /* Get device */
    adev = anarchy_get_device();
    if (!adev) {
        pr_err("Failed to get device\n");
        ret = -ENODEV;
        goto err_exit;
    }

    /* Run tests */
    pr_info("Running ring transfer tests\n");
    ret = test_ring_stress(adev);
    if (ret)
        goto err_exit;

    pr_info("All tests passed!\n");
    return 0;

err_exit:
    anarchy_tb_exit();
err_free:
    kfree(test_data);
    kfree(recv_data);
    return ret;
}

static void __exit test_exit(void)
{
    anarchy_tb_exit();
    kfree(test_data);
    kfree(recv_data);
    pr_info("Anarchy eGPU e2e tests completed\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("Anarchy eGPU End-to-End Tests"); 