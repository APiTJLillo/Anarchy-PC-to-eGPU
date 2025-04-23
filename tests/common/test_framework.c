#include "test_framework.h"
#include "anarchy-egpu.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "utils/test_utils.h"

struct test_context *create_test_context(void) {
    struct test_context *ctx = malloc(sizeof(struct test_context));
    if (!ctx)
        return NULL;
    
    ctx->data = NULL;
    ctx->status = 0;
    ctx->error_message = NULL;
    return ctx;
}

void free_test_context(struct test_context *ctx) {
    if (!ctx)
        return;
    
    if (ctx->error_message)
        free(ctx->error_message);
    free(ctx);
}

void inject_error(struct anarchy_device *dev, struct error_injection_params *params) {
    if (!dev || !params)
        return;

    switch (params->type) {
        case ERR_DMA_TIMEOUT:
            dev->error_state = ANARCHY_ERR_DMA_TIMEOUT;
            break;
        case ERR_COMMAND_TIMEOUT:
            dev->error_state = ANARCHY_ERR_CMD_TIMEOUT;
            break;
        case ERR_GPU_FAULT:
            dev->error_state = ANARCHY_ERR_GPU_FAULT;
            break;
        case ERR_MEMORY_ERROR:
            dev->error_state = ANARCHY_ERR_MEMORY;
            break;
    }

    if (params->delay > 0)
        msleep(params->delay);
}

int check_gpu_computation_state(struct anarchy_device *dev) {
    if (!dev)
        return -EINVAL;
    return dev->gpu_computation_active ? 1 : COMP_STATE_STOPPED;
}

int check_memory_transfer_state(struct anarchy_device *dev) {
    if (!dev)
        return -EINVAL;
    return dev->dma_active ? 1 : TRANSFER_STATE_STOPPED;
}

int check_command_processor_state(struct anarchy_device *dev) {
    if (!dev)
        return -EINVAL;
    return dev->command_processor_active ? 1 : CMD_STATE_STOPPED;
}

int check_resource_leaks(struct anarchy_device *dev) {
    if (!dev)
        return -EINVAL;
    
    // Check for various resource leaks
    if (dev->dma_active || 
        dev->gpu_computation_active || 
        dev->command_processor_active ||
        dev->ref_count > 1)
        return 1;
    
    return 0;
}

char *generate_test_data(size_t size) {
    char *data = malloc(size);
    if (!data)
        return NULL;
    
    // Fill with pseudo-random data
    for (size_t i = 0; i < size; i++) {
        data[i] = (char)(get_random_u32() & 0xFF);
    }
    return data;
}

/* Implementation moved to test_runner.c */
