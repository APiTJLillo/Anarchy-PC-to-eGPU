#include "anarchy-device.h"
#include <stdlib.h>
#include <string.h>

struct anarchy_device *init_test_device(void) {
    struct anarchy_device *dev = malloc(sizeof(struct anarchy_device));
    if (!dev)
        return NULL;
    
    memset(dev, 0, sizeof(struct anarchy_device));
    dev->ref_count = 1;
    return dev;
}

void cleanup_test_device(struct anarchy_device *dev) {
    if (!dev)
        return;
    free(dev);
}

int simulate_connection(struct anarchy_device *dev) {
    if (!dev)
        return -1;
    
    dev->connected = true;
    dev->error_state = ANARCHY_ERR_NONE;
    return 0;
}

int simulate_disconnection(struct anarchy_device *dev) {
    if (!dev)
        return -1;
    
    dev->connected = false;
    dev->error_state = ANARCHY_ERR_DISCONNECTED;
    dev->dma_active = false;
    dev->gpu_computation_active = false;
    dev->command_processor_active = false;
    return 0;
}

struct transfer_context *begin_async_transfer(struct anarchy_device *dev, void *data, size_t size) {
    if (!dev || !data || !size)
        return NULL;
        
    struct transfer_context *ctx = malloc(sizeof(struct transfer_context));
    if (!ctx)
        return NULL;
        
    ctx->data = malloc(size);
    if (!ctx->data) {
        free(ctx);
        return NULL;
    }
    
    memcpy(ctx->data, data, size);
    ctx->size = size;
    ctx->completed = false;
    dev->dma_active = true;
    
    return ctx;
}

void cleanup_transfer(struct transfer_context *transfer) {
    if (!transfer)
        return;
    
    if (transfer->data)
        free(transfer->data);
    free(transfer);
}

void start_gpu_computation(struct anarchy_device *dev) {
    if (!dev)
        return;
    dev->gpu_computation_active = true;
}

void start_memory_transfer(struct anarchy_device *dev) {
    if (!dev)
        return;
    dev->dma_active = true;
}

void start_command_processing(struct anarchy_device *dev) {
    if (!dev)
        return;
    dev->command_processor_active = true;
}
