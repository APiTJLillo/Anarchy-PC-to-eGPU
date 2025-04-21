#ifndef ANARCHY_EGPU_H
#define ANARCHY_EGPU_H

#include <linux/types.h>
#include <linux/thunderbolt.h>

/* Forward declarations */
struct anarchy_device;

/* Thunderbolt service initialization/cleanup */
int anarchy_tb_init(void);
void anarchy_tb_exit(void);

/* Error handling */
void anarchy_handle_connection_error(struct anarchy_device *adev, int err);

/* Ring buffer parameters */
#define ANARCHY_NUM_BUFFERS 8
#define ANARCHY_RING_SIZE (64 * 1024)  /* 64KB per buffer */

/* Ring states */
enum ring_state {
    RING_STATE_STOPPED = 0,
    RING_STATE_RUNNING,
};

/* Frame structure */
struct anarchy_frame {
    void *data;
    dma_addr_t dma;
    size_t size;
    u32 flags;
};

/* Ring structure */
struct anarchy_ring {
    struct anarchy_device *adev;
    struct tb_ring *ring;
    struct anarchy_frame frames[ANARCHY_NUM_BUFFERS];
    struct ring_frame frame;
    struct anarchy_transfer *current_transfer;
    atomic_t pending;
    spinlock_t lock;
    wait_queue_head_t wait;
    enum ring_state state;
};

/* Transfer structure */
struct anarchy_transfer {
    void *data;
    size_t size;
    u32 flags;
    bool completed;
    int error;
};

/* Device structure */
struct anarchy_device {
    struct tb_service *service;
    struct device *dev;
    struct tb_nhi *nhi;
    struct workqueue_struct *wq;
    struct work_struct recovery_work;
    struct anarchy_ring tx_ring;
    struct anarchy_ring rx_ring;
    u32 max_lanes;
    u32 max_speed;
};

/* Function declarations */
struct anarchy_device *anarchy_get_device(void);
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                          struct anarchy_transfer *transfer);

#endif /* ANARCHY_EGPU_H */ 