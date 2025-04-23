#ifndef __ANARCHY_RING_H__
#define __ANARCHY_RING_H__

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

struct anarchy_device;
struct dma_ring;

/* Ring buffer states */
enum anarchy_ring_state {
    ANARCHY_RING_STATE_STOPPED,
    ANARCHY_RING_STATE_RUNNING,
    ANARCHY_RING_STATE_ERROR
};

/* Transfer structure */
struct anarchy_transfer {
    void *buffer;
    size_t size;
    u32 flags;
};

/* Ring buffer structure */
struct anarchy_ring {
    struct anarchy_device *adev;
    enum anarchy_ring_state state;
    unsigned int head;
    unsigned int tail;
    bool is_tx;
    struct dma_ring *dma;
    spinlock_t lock;
    wait_queue_head_t wait;
    atomic_t bytes_transferred;
    atomic_t transfer_errors;
    atomic_t error_count;
    atomic_t pending;
    void *transfers;
};

/* Ring buffer functions */
int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring);
void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         void *data, size_t size, struct anarchy_transfer *transfer);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer);

#endif /* __ANARCHY_RING_H__ */
