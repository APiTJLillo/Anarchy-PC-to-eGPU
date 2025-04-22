#ifndef ANARCHY_RING_H
#define ANARCHY_RING_H

#include <linux/types.h>
#include <linux/thunderbolt.h>
#include "forward.h"

/* Ring states */
enum anarchy_ring_state {
    ANARCHY_RING_STATE_STOPPED = 0,
    ANARCHY_RING_STATE_RUNNING,
    ANARCHY_RING_STATE_ERROR
};

/* Transfer configuration */
struct anarchy_transfer {
    void *buffer;
    size_t size;
    u32 flags;
    int error;
    void *context;
    void (*callback)(void *context, int error);
};

/* Ring configuration */
struct anarchy_ring {
    struct anarchy_device *adev;
    struct tb_ring *ring;
    
    /* State tracking */
    enum anarchy_ring_state state;
    
    /* Ring buffer */
    void *buffer;
    size_t buffer_size;
    u32 head;
    u32 tail;
    
    /* Transfer queue */
    struct anarchy_transfer *transfers;
    unsigned int num_transfers;
    spinlock_t lock;
    
    /* Statistics */
    atomic_t bytes_transferred;
    atomic_t transfer_errors;
};

/* Ring management functions */
int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring);
void anarchy_ring_cleanup(struct anarchy_ring *ring);
int anarchy_ring_start(struct anarchy_ring *ring);
void anarchy_ring_stop(struct anarchy_ring *ring);
int anarchy_ring_submit(struct anarchy_ring *ring, struct anarchy_transfer *transfer);

#endif /* ANARCHY_RING_H */
    atomic_t error_count;
    atomic_t pending;
    spinlock_t lock;
    wait_queue_head_t wait;
    
    /* Transfer state */
    struct anarchy_transfer *current_transfer;
    struct ring_frame frame;
    bool is_tx;
    
    /* DMA buffers */
    struct {
        void *buffer;
        dma_addr_t buffer_phy;
        size_t size;
        u32 flags;
    } frames[32];
};

/* Ring lifecycle - external interface */
int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring);

/* Ring operations - internal interface */
void anarchy_ring_error(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring, 
                         struct anarchy_transfer *transfer);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer);

#endif /* ANARCHY_RING_H */
