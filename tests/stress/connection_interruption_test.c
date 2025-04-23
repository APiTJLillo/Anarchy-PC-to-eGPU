#include "test_framework.h"
#include "anarchy-egpu.h"
#include "anarchy-device.h"
#include "../common/utils/test_utils.h"

static struct anarchy_device *dev;
static struct test_context *ctx;

static void setup_test_env(void) {
    dev = init_test_device();
    ctx = create_test_context();
    REQUIRE(dev != NULL);
    REQUIRE(ctx != NULL);
}

static void cleanup_test_env(void) {
    if (dev) cleanup_test_device(dev);
    if (ctx) free_test_context(ctx);
}

void before_each(void) {
    setup_test_env();
}

void after_each(void) {
    cleanup_test_env();
}

void test_rapid_connect_disconnect(void) {
    for (int i = 0; i < 100; i++) {
        REQUIRE(simulate_connection(dev) == 0);
        REQUIRE(dev->connected == true);
        msleep(10); // Brief delay
        REQUIRE(simulate_disconnection(dev) == 0);
        REQUIRE(dev->connected == false);
        msleep(10);
    }
}

void test_disconnect_during_transfer(void) {
    const int transfer_size = 1024 * 1024; // 1MB
    char *test_data = generate_test_data(transfer_size);
    
    for (int i = 0; i < 50; i++) {
        REQUIRE(simulate_connection(dev) == 0);
        // Start async data transfer
        struct transfer_context *transfer = begin_async_transfer(dev, test_data, transfer_size);
        REQUIRE(transfer != NULL);
        
        // Wait for transfer to begin
        msleep(5);
        
        // Simulate sudden disconnection
        REQUIRE(simulate_disconnection(dev) == 0);
        
        // Verify cleanup and recovery
        REQUIRE(dev->error_state == ANARCHY_ERR_DISCONNECTED);
        REQUIRE(dev->dma_active == false);
        
        // Attempt reconnection
        REQUIRE(simulate_connection(dev) == 0);
        REQUIRE(dev->connected == true);
        REQUIRE(dev->error_state == ANARCHY_ERR_NONE);
        
        cleanup_transfer(transfer);
        msleep(50); // Cool-down period
    }
    
    free(test_data);
}

void test_multi_operation_interruption(void) {
    for (int i = 0; i < 30; i++) {
        // Setup initial connection
        REQUIRE(simulate_connection(dev) == 0);
        
        // Start multiple operations
        start_gpu_computation(dev);
        start_memory_transfer(dev);
        start_command_processing(dev);
        
        // Random delay between 1-100ms
        msleep(get_random_u32() % 100 + 1);
        
        // Sudden disconnection
        REQUIRE(simulate_disconnection(dev) == 0);
        
        // Verify all operations stopped safely
        REQUIRE(check_gpu_computation_state(dev) == COMP_STATE_STOPPED);
        REQUIRE(check_memory_transfer_state(dev) == TRANSFER_STATE_STOPPED);
        REQUIRE(check_command_processor_state(dev) == CMD_STATE_STOPPED);
        
        // Verify no memory leaks or hanging operations
        REQUIRE(check_resource_leaks(dev) == 0);
        
        msleep(100); // Cool-down period
    }
}

void test_reconnection_after_errors(void) {
    struct error_injection_params errors[] = {
        {.type = ERR_DMA_TIMEOUT, .delay = 10},
        {.type = ERR_COMMAND_TIMEOUT, .delay = 20},
        {.type = ERR_GPU_FAULT, .delay = 15},
        {.type = ERR_MEMORY_ERROR, .delay = 25}
    };
    
    for (int i = 0; i < ARRAY_SIZE(errors); i++) {
        // Connect and inject error
        REQUIRE(simulate_connection(dev) == 0);
        inject_error(dev, &errors[i]);
        
        // Verify error handling
        REQUIRE(dev->error_state != ANARCHY_ERR_NONE);
        
        // Simulate disconnection during error state
        REQUIRE(simulate_disconnection(dev) == 0);
        
        // Attempt recovery through reconnection
        REQUIRE(simulate_connection(dev) == 0);
        REQUIRE(dev->connected == true);
        REQUIRE(dev->error_state == ANARCHY_ERR_NONE);
        
        msleep(100); // Cool-down period
    }
}

/* Register all test functions */
REGISTER_TEST(rapid_connect_disconnect, test_rapid_connect_disconnect);
REGISTER_TEST(disconnect_during_transfer, test_disconnect_during_transfer);
REGISTER_TEST(multi_operation_interruption, test_multi_operation_interruption);
REGISTER_TEST(reconnection_after_errors, test_reconnection_after_errors);

/* Main function to run all tests */
int main(void) {
    run_all_tests();
    return 0;
}
