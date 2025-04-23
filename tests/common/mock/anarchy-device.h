#ifndef MOCK_DEVICE_H
#define MOCK_DEVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Basic type definitions for testing */
typedef uint32_t u32;
typedef uint64_t u64;

/* Mock device states and errors */
#define ANARCHY_ERR_NONE         0
#define ANARCHY_ERR_DISCONNECTED 1
#define ANARCHY_ERR_DMA_TIMEOUT  2
#define ANARCHY_ERR_CMD_TIMEOUT  3
#define ANARCHY_ERR_GPU_FAULT    4
#define ANARCHY_ERR_MEMORY       5

/* Mock device structure */
struct anarchy_device {
    bool connected;
    int error_state;
    bool dma_active;
    bool gpu_computation_active;
    bool command_processor_active;
    int ref_count;
};

/* Mock device functions */
struct anarchy_device *init_test_device(void);
void cleanup_test_device(struct anarchy_device *dev);
int simulate_connection(struct anarchy_device *dev);
int simulate_disconnection(struct anarchy_device *dev);

struct transfer_context {
    void *data;
    size_t size;
    bool completed;
};

struct transfer_context *begin_async_transfer(struct anarchy_device *dev, 
                                           void *data, 
                                           size_t size);
void cleanup_transfer(struct transfer_context *transfer);

/* Mock operations */
void start_gpu_computation(struct anarchy_device *dev);
void start_memory_transfer(struct anarchy_device *dev);
void start_command_processing(struct anarchy_device *dev);

#endif /* MOCK_DEVICE_H */
