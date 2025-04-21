#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/thunderbolt.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include "anarchy-egpu.h"
#include "pcie.h"

/* Ring synchronization */
static DEFINE_MUTEX(ring_mutex);
static DECLARE_WAIT_QUEUE_HEAD(xdomain_wait);

/* Global device pointer for tests */
static struct anarchy_device *global_adev;
static atomic_t init_count = ATOMIC_INIT(0);
static DEFINE_MUTEX(init_mutex);

/* USB4 and PCIe state definitions */
enum tb_usb4_state {
    TB_USB4_STATE_UNKNOWN = 0,
    TB_USB4_STATE_CONNECTING,
    TB_USB4_STATE_CONNECTED,
    TB_USB4_STATE_TRAINING,
    TB_USB4_STATE_ERROR,
    TB_USB4_STATE_DISABLED
};

enum tb_pcie_state {
    TB_PCIE_STATE_UNKNOWN = 0,
    TB_PCIE_LINK_DOWN,
    TB_PCIE_TRAINING,
    TB_PCIE_OK,
    TB_PCIE_ERROR,
    TB_PCIE_POWER_CHANGE,
    TB_PCIE_USB4_NEGOTIATION,
    TB_PCIE_MODE_SWITCHING,
    TB_PCIE_HOST_RECONNECT,
    TB_PCIE_LINK_UP
};

/* Connection parameters */
#define TB_USB4_MAX_RETRIES 5
#define TB_USB4_BACKOFF_BASE_MS 1000
#define TB_USB4_STABILITY_TIMEOUT_MS 5000
#define TB_USB4_MIN_STABLE_TIME_MS 5000
#define TB_USB4_RECONNECT_DELAY_MS 3000
#define TB_USB4_BACKOFF_MAX_MS 30000
#define TB_USB4_MAX_QUICK_RECONNECTS 3
#define TB_USB4_STABLE_WINDOW_MS 15000
#define TB_USB4_QUICK_RECONNECT_MS 2000
#define TB_USB4_RETRY_DELAY_MS 1000
#define TB_USB4_MAX_RECOVERY_RETRIES 3
#define TB_RING_ERROR_THRESHOLD 10

/* Microsoft USB4 Connection Manager specific parameters */
#define MS_USB4_VENDOR_ID 0x45e
#define MS_USB4_DEVICE_ID 0x83f
#define MS_USB4_MIN_STABLE_TIME_MS 10000

/* Connection state parameters */
#define TB_CONNECTION_DEBOUNCE_MS     1000
#define TB_MAX_RECONNECT_ATTEMPTS     5
#define TB_USB4_MIN_CONNECT_TIME_MS   2000
#define TB_USB4_MAX_TRANSITIONS       10

/* Ring buffer parameters */
#define ANARCHY_RING_SIZE             (64 * 1024)
#define ANARCHY_NUM_BUFFERS           8
#define ANARCHY_MAX_RETRIES 3
#define ANARCHY_RETRY_DELAY_MS 1000

/* XDomain parameters */
#define TB_XDOMAIN_REQ_SIZE 256
#define TB_XDOMAIN_TIMEOUT_MS 15000

/* Function prototypes */
static void anarchy_ring_frame_callback(struct tb_ring *ring, struct ring_frame *frame,
                                      bool canceled);
static int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring);
static void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring);
static void anarchy_ring_error(struct anarchy_device *adev, struct anarchy_ring *ring);
static void anarchy_connection_recovery_work(struct work_struct *work);
static int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id);
static void anarchy_service_remove(struct tb_service *svc);
static void update_pcie_state(struct anarchy_device *adev, enum tb_pcie_state new_state,
                            const char *reason);
static void log_pcie_error(struct anarchy_device *adev, const char *error_msg, int status);
static bool anarchy_check_usb4_stability(struct tb_service *svc);
int anarchy_pcie_train_link(struct anarchy_device *adev);
void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                          struct anarchy_transfer *transfer);
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer);
int anarchy_pcie_wait_for_link(struct anarchy_device *adev, u32 *link_status);

/* State tracking structure */
struct tb_state_info {
    /* PCIe state */
    enum tb_pcie_state pcie_state;
    unsigned long pcie_errors;
    unsigned long training_attempts;
    unsigned long successful_trains;
    unsigned int current_speed;
    unsigned int current_width;
    char last_pcie_error[128];

    /* USB4/Connection state */
    enum tb_usb4_state usb4_state;
    unsigned long last_state_change;
    unsigned long connection_attempts;
    unsigned long successful_connects;
    unsigned long failed_connects;
    unsigned long total_connected_time;
    unsigned long last_connect;
    unsigned long last_disconnect;
    unsigned long stable_since;
    unsigned int reconnect_count;
    unsigned int quick_reconnect_count;
    unsigned long backoff_time;
    bool in_backoff;
    bool is_stable;
    char last_usb4_error[128];

    /* Ring buffer stats */
    unsigned long ring_errors;
    unsigned long successful_transfers;
    unsigned long failed_transfers;
    unsigned long total_bytes_transferred;
    char last_ring_error[128];

    /* Synchronization */
    spinlock_t lock;
};

/* Global state info instance */
static struct tb_state_info state_info = {
    .pcie_state = TB_PCIE_STATE_UNKNOWN,
    .usb4_state = TB_USB4_STATE_UNKNOWN,
    .is_stable = false,
    .in_backoff = false,
    .lock = __SPIN_LOCK_UNLOCKED(state_info.lock),
};

/* XDomain request parameters */
static u8 __maybe_unused xdomain_req_buffer[TB_XDOMAIN_REQ_SIZE];
static u8 __maybe_unused xdomain_resp_buffer[TB_XDOMAIN_REQ_SIZE];

/* Service table */
static const struct tb_service_id anarchy_service_table[] = {
    {
        .match_flags = TBSVC_MATCH_PROTOCOL_KEY |
                      TBSVC_MATCH_PROTOCOL_ID,
        .protocol_key = 0x34,
        .protocol_id = 0x5678
    },
    { }  /* sentinel */
};

MODULE_DEVICE_TABLE(tbsvc, anarchy_service_table);

/* Service driver structure */
static struct tb_service_driver anarchy_service_driver;

/* Return the PCIe NHI that owns this xdomain (kernel ≥ 6.10) */
static struct tb_nhi *get_nhi_from_xdomain(struct tb_xdomain *xd)
{
    /* Sanity checks are cheap and prevent crashes */
    if (!xd || !xd->tb)
        return NULL;

    return xd->tb->nhi;
}

static bool is_ms_usb4_manager(struct tb_service *svc)
{
    struct tb_xdomain *xd = tb_service_parent(svc);
    if (!xd)
        return false;
    
    return (xd->vendor == MS_USB4_VENDOR_ID && xd->device == MS_USB4_DEVICE_ID);
}

static bool should_attempt_connection(struct tb_service *svc)
{
    unsigned long now = jiffies;
    bool should_connect = true;
    unsigned long min_wait;
    
    spin_lock(&state_info.lock);
    
    /* Check if we're in backoff period */
    if (state_info.in_backoff) {
        if (time_before(now, state_info.last_disconnect + msecs_to_jiffies(state_info.backoff_time))) {
            dev_info(&svc->dev, "In backoff period, waiting %lu ms\n", state_info.backoff_time);
            should_connect = false;
            goto out_unlock;
        }
        state_info.in_backoff = false;
    }
    
    /* Check for quick reconnects */
    if (state_info.last_disconnect && 
        time_before(now, state_info.last_disconnect + msecs_to_jiffies(TB_USB4_MIN_CONNECT_TIME_MS))) {
        state_info.quick_reconnect_count++;
        
        if (state_info.quick_reconnect_count > TB_USB4_MAX_QUICK_RECONNECTS) {
            /* Enter exponential backoff */
            state_info.backoff_time = min(state_info.backoff_time * 2, TB_USB4_BACKOFF_MAX_MS);
            state_info.in_backoff = true;
            dev_warn(&svc->dev, "Too many quick reconnects, backing off for %lu ms\n",
                    state_info.backoff_time);
            should_connect = false;
            goto out_unlock;
        }
    } else {
        /* Reset quick reconnect counter if enough time has passed */
        state_info.quick_reconnect_count = 0;
        state_info.backoff_time = TB_USB4_BACKOFF_BASE_MS;
    }
    
    /* Enforce minimum wait time between connection attempts */
    min_wait = state_info.quick_reconnect_count ? 
               TB_USB4_BACKOFF_BASE_MS * (1 << state_info.quick_reconnect_count) :
               TB_USB4_BACKOFF_BASE_MS;
               
    if (state_info.last_disconnect && 
        time_before(now, state_info.last_disconnect + msecs_to_jiffies(min_wait))) {
        dev_info(&svc->dev, "Enforcing minimum wait time of %lu ms\n", min_wait);
        should_connect = false;
    }

out_unlock:
    spin_unlock(&state_info.lock);
    return should_connect;
}

static void __maybe_unused update_connection_stats(struct tb_service *svc, bool connected)
{
    unsigned long now = jiffies;
    unsigned long connect_duration = 0;
    unsigned long flags;
    bool was_stable;
    
    spin_lock_irqsave(&state_info.lock, flags);
    
    was_stable = state_info.is_stable;
    
    if (connected) {
        state_info.last_connect = now;
        state_info.connection_attempts++;
        
        /* Reset quick reconnect counter if enough time has passed */
        if (!state_info.last_disconnect || 
            time_after(now, state_info.last_disconnect + 
                      msecs_to_jiffies(TB_USB4_STABLE_WINDOW_MS))) {
            state_info.quick_reconnect_count = 0;
            state_info.backoff_time = TB_USB4_BACKOFF_BASE_MS;
        }
        
        state_info.stable_since = now;
        state_info.successful_connects++;
    } else {
        state_info.last_disconnect = now;
        state_info.is_stable = false;
        
        if (state_info.last_connect) {
            connect_duration = jiffies_to_msecs(now - state_info.last_connect);
            state_info.total_connected_time += connect_duration;
            
            /* Check if this was a quick disconnect */
            if (connect_duration < TB_USB4_MIN_CONNECT_TIME_MS) {
                state_info.quick_reconnect_count++;
                
                /* Increase backoff time for frequent quick disconnects */
                if (state_info.quick_reconnect_count > TB_USB4_MAX_QUICK_RECONNECTS) {
                    state_info.backoff_time = min(state_info.backoff_time * 2, 
                                                (unsigned long)TB_USB4_BACKOFF_MAX_MS);
                    state_info.in_backoff = true;
                }
            }
        }
    }
    
    spin_unlock_irqrestore(&state_info.lock, flags);
    
    /* Log significant state changes */
    if (connected) {
        dev_info(&svc->dev, "USB4 connection established (attempt %lu)\n",
                 state_info.connection_attempts);
    } else if (was_stable) {
        dev_info(&svc->dev, "Stable USB4 connection lost after %lu ms\n",
                 connect_duration);
    }
}

static void anarchy_ring_frame_callback(struct tb_ring *ring, struct ring_frame *frame,
                                      bool canceled)
{
    struct anarchy_ring *aring = container_of(frame, struct anarchy_ring, frame);
    struct anarchy_device *adev = aring->adev;
    unsigned long flags;
    
    if (!canceled) {
        spin_lock_irqsave(&state_info.lock, flags);
        state_info.successful_transfers++;
        state_info.total_bytes_transferred += frame->size;
        spin_unlock_irqrestore(&state_info.lock, flags);
    } else {
        spin_lock_irqsave(&state_info.lock, flags);
        state_info.failed_transfers++;
        spin_unlock_irqrestore(&state_info.lock, flags);
    }
    
    anarchy_ring_complete(adev, aring, aring->current_transfer);
}

static int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    int i;

    spin_lock_init(&ring->lock);
    init_waitqueue_head(&ring->wait);
    ring->state = RING_STATE_STOPPED;
    atomic_set(&ring->pending, 0);
    ring->current_transfer = NULL;

    for (i = 0; i < ANARCHY_NUM_BUFFERS; i++) {
        ring->frames[i].data = NULL;
        ring->frames[i].dma = 0;
        ring->frames[i].size = 0;
        ring->frames[i].flags = 0;
    }

    spin_lock(&state_info.lock);
    state_info.ring_errors = 0;
    state_info.successful_transfers = 0;
    state_info.failed_transfers = 0;
    state_info.total_bytes_transferred = 0;
    memset(state_info.last_ring_error, 0, sizeof(state_info.last_ring_error));
    spin_unlock(&state_info.lock);

    return 0;
}

static void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    int i;

    for (i = 0; i < ANARCHY_NUM_BUFFERS; i++) {
        if (ring->frames[i].data) {
            dma_free_coherent(&adev->service->dev,
                            ANARCHY_RING_SIZE,
                            ring->frames[i].data,
                            ring->frames[i].dma);
            ring->frames[i].data = NULL;
            ring->frames[i].dma = 0;
        }
    }
}

int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx)
{
    int ret = 0;
    int i;
    struct tb_nhi *nhi;

    if (ring->state != RING_STATE_STOPPED)
        return -EINVAL;

    dev_info(adev->dev, "Starting %s ring initialization\n", tx ? "TX" : "RX");

    nhi = adev->nhi;
    if (!nhi) {
        dev_err(adev->dev, "No NHI device available\n");
        return -ENODEV;
    }

    // Allocate ring buffers
    for (i = 0; i < ANARCHY_NUM_BUFFERS; i++) {
        ring->frames[i].data = dma_alloc_coherent(&adev->service->dev,
                                           ANARCHY_RING_SIZE,
                                           &ring->frames[i].dma,
                                           GFP_KERNEL);
        if (!ring->frames[i].data) {
            ret = -ENOMEM;
            goto cleanup;
        }
        ring->frames[i].size = ANARCHY_RING_SIZE;
    }

    // Allocate Thunderbolt ring
    if (tx)
        ring->ring = tb_ring_alloc_tx(nhi, 0, ANARCHY_NUM_BUFFERS, 0);
    else
        ring->ring = tb_ring_alloc_rx(nhi, 0, ANARCHY_NUM_BUFFERS, 0, 0,
                                    0xFFFF, 0xFFFF, NULL, NULL);

    if (!ring->ring) {
        ret = -ENOMEM;
        goto cleanup;
    }

    tb_ring_start(ring->ring);
    ring->state = RING_STATE_RUNNING;
    dev_info(adev->dev, "anarchy: %s ring started\n", tx ? "TX" : "RX");
    return 0;

cleanup:
    anarchy_ring_cleanup(adev, ring);
    dev_err(adev->dev, "anarchy: %s ring error: %d\n", tx ? "TX" : "RX", ret);
    return ret;
}

void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring)
{
    if (ring->state != RING_STATE_RUNNING)
        return;

    ring->state = RING_STATE_STOPPED;
    if (ring->ring) {
        tb_ring_stop(ring->ring);
        tb_ring_free(ring->ring);
        ring->ring = NULL;
    }

    anarchy_ring_cleanup(adev, ring);
}

int anarchy_ring_transfer(struct anarchy_device *adev, struct anarchy_ring *ring,
                         struct anarchy_transfer *transfer)
{
    unsigned long flags;
    int ret = 0;
    int buf_idx;

    if (!transfer || !transfer->data || !transfer->size ||
        transfer->size > ANARCHY_RING_SIZE)
        return -EINVAL;

    spin_lock_irqsave(&ring->lock, flags);

    if (ring->state != RING_STATE_RUNNING) {
        ret = -EINVAL;
        goto out_unlock;
    }

    buf_idx = atomic_read(&ring->pending) % ANARCHY_NUM_BUFFERS;
    
    memcpy(ring->frames[buf_idx].data, transfer->data, transfer->size);
    ring->frames[buf_idx].size = transfer->size;
    ring->frames[buf_idx].flags = transfer->flags;

    ring->current_transfer = transfer;
    atomic_inc(&ring->pending);

    ring->frame.buffer_phy = ring->frames[buf_idx].dma;
    ring->frame.size = transfer->size;
    ring->frame.flags = transfer->flags;
    ring->frame.callback = anarchy_ring_frame_callback;

    ret = tb_ring_tx(ring->ring, &ring->frame);
    if (ret)
        anarchy_ring_error(adev, ring);

out_unlock:
    spin_unlock_irqrestore(&ring->lock, flags);
    return ret;
}

void anarchy_ring_complete(struct anarchy_device *adev, struct anarchy_ring *ring,
                          struct anarchy_transfer *transfer)
{
    unsigned long flags;

    spin_lock_irqsave(&ring->lock, flags);
    if (ring->current_transfer == transfer) {
        ring->current_transfer = NULL;
        atomic_dec(&ring->pending);
        wake_up(&ring->wait);
    }
    spin_unlock_irqrestore(&ring->lock, flags);
}

void anarchy_handle_connection_error(struct anarchy_device *adev, int err)
{
    unsigned long flags;
    unsigned long now = jiffies;
    bool schedule_recovery = true;
    
    if (!adev)
        return;

    spin_lock_irqsave(&state_info.lock, flags);
    
    /* Update error statistics */
    state_info.failed_connects++;
    
    /* Check for quick reconnects */
    if (state_info.last_disconnect &&
        time_before(now, state_info.last_disconnect + msecs_to_jiffies(TB_USB4_QUICK_RECONNECT_MS))) {
        state_info.quick_reconnect_count++;
    } else {
        state_info.quick_reconnect_count = 0;
    }
    
    /* Update backoff timing */
    if (state_info.in_backoff) {
        /* Exponential backoff up to max */
        state_info.backoff_time = min_t(unsigned long,
                                      state_info.backoff_time * 2,
                                      TB_USB4_BACKOFF_MAX_MS);
    } else {
        state_info.in_backoff = true;
        state_info.backoff_time = TB_USB4_BACKOFF_BASE_MS;
    }
    
    /* Update state */
    state_info.usb4_state = TB_USB4_STATE_ERROR;
    state_info.last_state_change = now;
    state_info.last_disconnect = now;
    state_info.is_stable = false;
    
    /* Log detailed error info */
    snprintf(state_info.last_usb4_error, sizeof(state_info.last_usb4_error),
             "Connection error %d (PCIe: %s, attempts: %lu, backoff: %lu ms)",
             err,
             state_info.last_pcie_error,
             state_info.connection_attempts,
             state_info.backoff_time);
    
    dev_err(adev->dev, "%s\n", state_info.last_usb4_error);
    
    /* Don't schedule recovery if we're in extended backoff */
    if (state_info.quick_reconnect_count > TB_USB4_MAX_QUICK_RECONNECTS) {
        dev_info(adev->dev, "Too many quick reconnects (%u), waiting %lu ms\n",
                state_info.quick_reconnect_count, state_info.backoff_time);
        schedule_recovery = false;
    }
    
    spin_unlock_irqrestore(&state_info.lock, flags);
    
    /* Schedule recovery work if needed */
    if (schedule_recovery && adev->wq)
        queue_work(adev->wq, &adev->recovery_work);
}

static void anarchy_connection_recovery_work(struct work_struct *work)
{
    struct anarchy_device *adev = container_of(work, struct anarchy_device, recovery_work);
    struct tb_xdomain *xd;
    unsigned long flags;
    unsigned long start_time;
    int retries = 0;
    int ret;

    /* Update state to indicate recovery in progress */
    spin_lock_irqsave(&state_info.lock, flags);
    state_info.usb4_state = TB_USB4_STATE_CONNECTING;
    state_info.connection_attempts++;
    start_time = jiffies;
    spin_unlock_irqrestore(&state_info.lock, flags);

    /* Stop rings before recovery */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);

    /* Apply backoff if needed */
    if (state_info.in_backoff) {
        dev_info(adev->dev, "Applying backoff delay of %lu ms\n", state_info.backoff_time);
        msleep(state_info.backoff_time);
    }

    do {
        /* Check if we should attempt recovery */
        xd = tb_service_parent(adev->service);
        if (!xd || !should_attempt_connection(adev->service)) {
            dev_warn(adev->dev, "Recovery skipped - connection not ready\n");
            goto recovery_failed;
        }

        /* Attempt PCIe link recovery first */
        ret = anarchy_pcie_train_link(adev);
        if (ret) {
            spin_lock_irqsave(&state_info.lock, flags);
            snprintf(state_info.last_usb4_error, sizeof(state_info.last_usb4_error),
                     "PCIe link training failed during recovery: %d", ret);
            spin_unlock_irqrestore(&state_info.lock, flags);
            dev_warn(adev->dev, "%s\n", state_info.last_usb4_error);
            goto retry;
        }

        /* Initialize rings */
        ret = anarchy_ring_init(adev, &adev->tx_ring);
        if (ret) {
            spin_lock_irqsave(&state_info.lock, flags);
            snprintf(state_info.last_usb4_error, sizeof(state_info.last_usb4_error),
                     "TX ring init failed during recovery: %d", ret);
            spin_unlock_irqrestore(&state_info.lock, flags);
            dev_warn(adev->dev, "%s\n", state_info.last_usb4_error);
            goto retry;
        }

        ret = anarchy_ring_init(adev, &adev->rx_ring);
        if (ret) {
            spin_lock_irqsave(&state_info.lock, flags);
            snprintf(state_info.last_usb4_error, sizeof(state_info.last_usb4_error),
                     "RX ring init failed during recovery: %d", ret);
            spin_unlock_irqrestore(&state_info.lock, flags);
            dev_warn(adev->dev, "%s\n", state_info.last_usb4_error);
            anarchy_ring_cleanup(adev, &adev->tx_ring);
            goto retry;
        }

        /* Start rings */
        ret = anarchy_ring_start(adev, &adev->tx_ring, true);
        if (ret) {
            spin_lock_irqsave(&state_info.lock, flags);
            snprintf(state_info.last_usb4_error, sizeof(state_info.last_usb4_error),
                     "TX ring start failed during recovery: %d", ret);
            spin_unlock_irqrestore(&state_info.lock, flags);
            dev_warn(adev->dev, "%s\n", state_info.last_usb4_error);
            goto cleanup_rings;
        }

        ret = anarchy_ring_start(adev, &adev->rx_ring, false);
        if (ret) {
            spin_lock_irqsave(&state_info.lock, flags);
            snprintf(state_info.last_usb4_error, sizeof(state_info.last_usb4_error),
                     "RX ring start failed during recovery: %d", ret);
            spin_unlock_irqrestore(&state_info.lock, flags);
            dev_warn(adev->dev, "%s\n", state_info.last_usb4_error);
            anarchy_ring_stop(adev, &adev->tx_ring);
            goto cleanup_rings;
        }

        /* Update state on successful recovery */
        spin_lock_irqsave(&state_info.lock, flags);
        state_info.usb4_state = TB_USB4_STATE_CONNECTED;
        state_info.last_connect = jiffies;
        state_info.successful_connects++;
        state_info.in_backoff = false;
        state_info.backoff_time = TB_USB4_BACKOFF_BASE_MS;
        state_info.quick_reconnect_count = 0;
        spin_unlock_irqrestore(&state_info.lock, flags);

        dev_info(adev->dev, "Connection recovery successful after %d retries\n", retries);
        return;

cleanup_rings:
        anarchy_ring_cleanup(adev, &adev->rx_ring);
        anarchy_ring_cleanup(adev, &adev->tx_ring);
retry:
        retries++;
        if (retries >= TB_USB4_MAX_RECOVERY_RETRIES) {
            dev_err(adev->dev, "Recovery failed after %d retries\n", retries);
            goto recovery_failed;
        }
        msleep(TB_USB4_RETRY_DELAY_MS);
    } while (retries < TB_USB4_MAX_RECOVERY_RETRIES);

recovery_failed:
    spin_lock_irqsave(&state_info.lock, flags);
    state_info.usb4_state = TB_USB4_STATE_ERROR;
    state_info.failed_connects++;
    state_info.in_backoff = true;
    state_info.backoff_time = min_t(unsigned long,
                                  state_info.backoff_time * 2,
                                  TB_USB4_BACKOFF_MAX_MS);
    spin_unlock_irqrestore(&state_info.lock, flags);

    dev_err(adev->dev, "Connection recovery failed after %d retries\n", retries);
}

static void anarchy_ring_error(struct anarchy_device *adev,
                             struct anarchy_ring *ring)
{
    unsigned long flags;
    unsigned int ms_since_connect;
    
    if (!adev || !ring)
        return;

    spin_lock_irqsave(&state_info.lock, flags);
    
    /* Update ring error statistics */
    state_info.ring_errors++;
    ms_since_connect = jiffies_to_msecs(jiffies - state_info.last_connect);
    
    snprintf(state_info.last_ring_error, sizeof(state_info.last_ring_error),
             "Ring %s error at %u ms after connect",
             ring == &adev->tx_ring ? "TX" : "RX",
             ms_since_connect);
    
    /* Update state if this is a critical error */
    if (state_info.ring_errors > TB_RING_ERROR_THRESHOLD) {
        state_info.usb4_state = TB_USB4_STATE_ERROR;
        state_info.pcie_state = TB_PCIE_ERROR;
        state_info.last_state_change = jiffies;
        dev_err(adev->dev, "Critical ring error: %s (errors: %lu, transfers: %lu/%lu)\n",
                state_info.last_ring_error,
                state_info.ring_errors,
                state_info.successful_transfers,
                state_info.failed_transfers);
        
        /* Schedule recovery work */
        if (adev->wq)
            queue_work(adev->wq, &adev->recovery_work);
    }
    
    spin_unlock_irqrestore(&state_info.lock, flags);
}

/* Service driver initialization */
int anarchy_tb_init(void)
{
    int ret;

    /* Initialize the service driver structure */
    anarchy_service_driver.driver.name = "anarchy";
    anarchy_service_driver.driver.owner = THIS_MODULE;
    anarchy_service_driver.id_table = anarchy_service_table;
    anarchy_service_driver.probe = anarchy_service_probe;
    anarchy_service_driver.remove = anarchy_service_remove;

    ret = tb_register_service_driver(&anarchy_service_driver);
    if (ret) {
        pr_err("Failed to register Anarchy eGPU service driver: %d\n", ret);
        return ret;
    }

    atomic_inc(&init_count);
    pr_info("Anarchy eGPU: Thunderbolt initialization complete\n");
    return 0;
}
EXPORT_SYMBOL_GPL(anarchy_tb_init);

/* Service driver cleanup */
void anarchy_tb_exit(void)
{
    if (atomic_read(&init_count) == 0) {
        pr_warn("Anarchy eGPU: Driver not initialized\n");
        return;
    }

    spin_lock_irq(&state_info.lock);
    state_info.usb4_state = TB_USB4_STATE_DISABLED;
    state_info.pcie_state = TB_PCIE_STATE_UNKNOWN;
    spin_unlock_irq(&state_info.lock);

    /* Ensure all work is stopped and cleaned up */
    if (global_adev && global_adev->wq) {
        cancel_work_sync(&global_adev->recovery_work);
        destroy_workqueue(global_adev->wq);
        global_adev->wq = NULL;
    }

    tb_unregister_service_driver(&anarchy_service_driver);
    atomic_set(&init_count, 0);
    pr_info("Anarchy eGPU: Thunderbolt cleanup complete\n");
}
EXPORT_SYMBOL_GPL(anarchy_tb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("Anarchy eGPU Thunderbolt Driver");
MODULE_VERSION("1.0");

/* PCIe state update function */
static void update_pcie_state(struct anarchy_device *adev, enum tb_pcie_state new_state,
                            const char *reason)
{
    unsigned long flags;
    enum tb_pcie_state old_state;

    spin_lock_irqsave(&state_info.lock, flags);
    old_state = state_info.pcie_state;
    state_info.pcie_state = new_state;
    state_info.last_state_change = jiffies;

    if (new_state == TB_PCIE_TRAINING) {
        state_info.training_attempts++;
        state_info.usb4_state = TB_USB4_STATE_TRAINING;
    } else if (new_state == TB_PCIE_OK) {
        state_info.successful_trains++;
        if (state_info.usb4_state == TB_USB4_STATE_TRAINING) {
            state_info.usb4_state = TB_USB4_STATE_CONNECTED;
            state_info.last_connect = jiffies;
            state_info.successful_connects++;
        }
    } else if (new_state == TB_PCIE_ERROR) {
        state_info.usb4_state = TB_USB4_STATE_ERROR;
        state_info.failed_connects++;
    }

    dev_info(adev->dev, "PCIe state change: %d -> %d (%s), USB4 state: %d\n",
             old_state, new_state, reason, state_info.usb4_state);
    spin_unlock_irqrestore(&state_info.lock, flags);
}

/* PCIe error logging function */
static void log_pcie_error(struct anarchy_device *adev, const char *error_msg, int status)
{
    unsigned long flags;
    
    spin_lock_irqsave(&state_info.lock, flags);
    state_info.pcie_errors++;
    strncpy(state_info.last_pcie_error, error_msg, sizeof(state_info.last_pcie_error) - 1);
    state_info.last_pcie_error[sizeof(state_info.last_pcie_error) - 1] = '\0';
    
    /* Update both PCIe and USB4 states on error */
    state_info.pcie_state = TB_PCIE_ERROR;
    state_info.usb4_state = TB_USB4_STATE_ERROR;
    state_info.last_state_change = jiffies;
    spin_unlock_irqrestore(&state_info.lock, flags);
    
    dev_err(adev->dev, "PCIe error: %s (status: %d)\n", error_msg, status);
}

/* PCIe link training implementation */
int anarchy_pcie_train_link(struct anarchy_device *adev)
{
    int ret;
    u32 link_status;
    unsigned long start_time = jiffies;

    update_pcie_state(adev, TB_PCIE_TRAINING, "Starting link training");

    /* Start link training */
    anarchy_pcie_retrain_link(adev);

    /* Wait for link to stabilize */
    ret = anarchy_pcie_wait_for_link(adev, &link_status);
    if (ret) {
        log_pcie_error(adev, "Link training timeout", ret);
        return ret;
    }

    /* Check final link status */
    if (!(link_status & PCI_EXP_LNKSTA_DLLLA)) {
        log_pcie_error(adev, "Link training failed - link not active", -ENOLINK);
        return -ENOLINK;
    }

    update_pcie_state(adev, TB_PCIE_OK, 
                     "Link training complete");
    
    dev_info(adev->dev, "Link training successful after %d ms\n",
             jiffies_to_msecs(jiffies - start_time));
    return 0;
}

/* Service probe implementation */
static int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id)
{
    struct anarchy_device *adev;
    int ret;
    unsigned long flags;
    struct tb_xdomain *xd;

    adev = devm_kzalloc(&svc->dev, sizeof(*adev), GFP_KERNEL);
    if (!adev)
        return -ENOMEM;

    adev->service = svc;
    adev->dev = &svc->dev;
    
    /* Get NHI from xdomain parent */
    xd = tb_service_parent(svc);
    if (!xd) {
        dev_err(&svc->dev, "No xdomain parent found\n");
        ret = -ENODEV;
        goto err_free;
    }
    
    /* Get NHI from parent device */
    adev->nhi = get_nhi_from_xdomain(xd);
    if (!adev->nhi) {
        dev_err(&svc->dev, "Failed to get NHI device\n");
        ret = -ENODEV;
        goto err_free;
    }

    /* Create single threaded workqueue for recovery */
    adev->wq = alloc_ordered_workqueue("anarchy_recovery", WQ_FREEZABLE | WQ_UNBOUND);
    if (!adev->wq) {
        dev_err(&svc->dev, "Failed to create recovery workqueue\n");
        ret = -ENOMEM;
        goto err_free;
    }

    INIT_WORK(&adev->recovery_work, anarchy_connection_recovery_work);

    /* Initialize consolidated state tracking atomically */
    spin_lock_irqsave(&state_info.lock, flags);
    memset(&state_info, 0, sizeof(state_info));
    state_info.pcie_state = TB_PCIE_STATE_UNKNOWN;
    state_info.usb4_state = TB_USB4_STATE_UNKNOWN;
    state_info.last_state_change = jiffies;
    state_info.backoff_time = TB_USB4_BACKOFF_BASE_MS;
    state_info.current_speed = 0;
    state_info.current_width = 0;
    state_info.connection_attempts = 0;
    state_info.successful_connects = 0;
    state_info.failed_connects = 0;
    state_info.total_connected_time = 0;
    state_info.ring_errors = 0;
    state_info.successful_transfers = 0;
    state_info.failed_transfers = 0;
    state_info.total_bytes_transferred = 0;
    spin_unlock_irqrestore(&state_info.lock, flags);

    /* Store device pointer */
    tb_service_set_drvdata(svc, adev);
    global_adev = adev;

    dev_info(&svc->dev, "Anarchy eGPU service initialized (PCIe max speed: Gen%d x%d)\n",
             adev->max_speed, adev->max_lanes);
    return 0;

err_free:
    if (adev->wq)
        destroy_workqueue(adev->wq);
    devm_kfree(&svc->dev, adev);
    return ret;
}

/* Improved USB4 stability check with consolidated state tracking */
static bool __maybe_unused anarchy_check_usb4_stability(struct tb_service *svc)
{
    struct tb_xdomain *xd = tb_service_parent(svc);
    unsigned long flags;
    bool is_stable = false;
    unsigned long now = jiffies;
    unsigned long min_stable_time;
    struct anarchy_device *adev = tb_service_get_drvdata(svc);

    if (!xd || !adev)
        return false;

    min_stable_time = is_ms_usb4_manager(svc) ? 
                     MS_USB4_MIN_STABLE_TIME_MS : 
                     TB_USB4_MIN_STABLE_TIME_MS;

    spin_lock_irqsave(&state_info.lock, flags);
    
    /* Check overall stability using consolidated state info */
    if (state_info.stable_since &&
        time_after(now, state_info.stable_since + msecs_to_jiffies(min_stable_time))) {
        
        /* Check all stability criteria at once */
        if (state_info.pcie_state == TB_PCIE_OK &&
            state_info.usb4_state == TB_USB4_STATE_CONNECTED &&
            state_info.ring_errors == 0 &&
            (!state_info.last_disconnect || 
             time_after(now, state_info.last_disconnect + 
                       msecs_to_jiffies(TB_USB4_STABLE_WINDOW_MS)))) {
            
            is_stable = true;
            
            /* Log comprehensive stability stats */
            dev_info(&svc->dev, 
                    "USB4 connection stable: PCIe Gen%d x%d, %u successful transfers, "
                    "%lu bytes transferred, uptime %d ms\n",
                    state_info.current_speed,
                    state_info.current_width,
                    (unsigned int)state_info.successful_transfers,
                    state_info.total_bytes_transferred,
                    jiffies_to_msecs(now - state_info.stable_since));
        }
    }
    
    /* Update stability state */
    if (!state_info.is_stable && is_stable) {
        state_info.is_stable = true;
        state_info.backoff_time = TB_USB4_BACKOFF_BASE_MS;
        state_info.in_backoff = false;
    }
    
    spin_unlock_irqrestore(&state_info.lock, flags);

    return is_stable;
}

/**
 * anarchy_get_device - Get the global device structure
 *
 * Returns the global device structure for testing.
 */
struct anarchy_device *anarchy_get_device(void)
{
	return global_adev;
}
EXPORT_SYMBOL_GPL(anarchy_get_device);

static void anarchy_service_remove(struct tb_service *svc)
{
    struct anarchy_device *adev = tb_service_get_drvdata(svc);
    unsigned long flags;
    
    if (!adev)
        return;

    /* Cancel pending work and destroy workqueue */
    cancel_work_sync(&adev->recovery_work);
    if (adev->wq) {
        destroy_workqueue(adev->wq);
        adev->wq = NULL;
    }

    /* Stop rings */
    anarchy_ring_stop(adev, &adev->tx_ring);
    anarchy_ring_stop(adev, &adev->rx_ring);

    /* Update state atomically */
    spin_lock_irqsave(&state_info.lock, flags);
    
    /* Log final statistics */
    dev_info(&svc->dev, 
             "Service removed. Stats: %u connects (%u successful, %u failed), "
             "%u PCIe errors, %u ring errors, %lu bytes transferred\n",
             (unsigned int)state_info.connection_attempts,
             (unsigned int)state_info.successful_connects,
             (unsigned int)state_info.failed_connects,
             (unsigned int)state_info.pcie_errors,
             (unsigned int)state_info.ring_errors,
             state_info.total_bytes_transferred);

    /* Clear state */
    state_info.usb4_state = TB_USB4_STATE_DISABLED;
    state_info.pcie_state = TB_PCIE_STATE_UNKNOWN;
    state_info.is_stable = false;
    state_info.in_backoff = false;
    
    spin_unlock_irqrestore(&state_info.lock, flags);

    dev_info(&svc->dev, "Anarchy eGPU service removed\n");
}
