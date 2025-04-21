#ifndef _ANARCHY_THUNDERBOLT_H_
#define _ANARCHY_THUNDERBOLT_H_

#include <linux/types.h>
#include <linux/thunderbolt.h>
#include "anarchy-egpu.h"
#include "pcie.h"

/* Thunderbolt service states */
enum tb_service_state {
    TB_SERVICE_DISCONNECTED = 0,
    TB_SERVICE_CONNECTING,
    TB_SERVICE_CONNECTED,
    TB_SERVICE_ERROR
};

/* Thunderbolt mode definitions */
#define TB_MODE_THUNDERBOLT 1
#define TB_MODE_USB4       2

/* Thunderbolt power states */
#define TB_SWITCH_POWER_ON  1
#define TB_SWITCH_POWER_OFF 0
#define TB_SWITCH_POWER_ERROR -1

/* Thunderbolt service structure */
struct anarchy_tb_service {
    struct tb_service *service;
    struct tb_nhi *nhi;
    struct anarchy_device *adev;
    enum tb_service_state state;
    struct work_struct recovery_work;
    atomic_t recovery_pending;
    int retry_count;
};

/* Thunderbolt helper functions */
static inline bool tb_router_is_accessible(struct tb_service *service)
{
    if (!service || !service->dev.parent)
        return false;
    return true;
}

static inline bool tb_switch_get_mode(struct tb_service *service)
{
    if (!service || !service->dev.parent)
        return false;
    return true;
}

static inline int tb_switch_set_mode(struct tb_service *service, int mode)
{
    if (!service || !service->dev.parent)
        return -EINVAL;
    return 0;
}

static inline int tb_switch_power_state(struct tb_service *service)
{
    if (!service || !service->dev.parent)
        return TB_SWITCH_POWER_ERROR;
    return TB_SWITCH_POWER_ON;
}

/* Function declarations */
int anarchy_tb_service_probe(struct tb_service *svc);
void anarchy_tb_service_remove(struct tb_service *svc);
int anarchy_tb_service_suspend(struct tb_service *svc);
int anarchy_tb_service_resume(struct tb_service *svc);
void anarchy_tb_service_shutdown(struct tb_service *svc);
int anarchy_tb_service_init(struct anarchy_device *adev);
void anarchy_tb_service_exit(struct anarchy_device *adev);
void anarchy_tb_service_recovery_work(struct work_struct *work);
int anarchy_tb_service_recovery(struct anarchy_device *adev);

/* Ring frame structure */
struct ring_frame {
    void *data;
    dma_addr_t dma;
    size_t size;
    u32 flags;
};

/* Service functions */
int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id);
void anarchy_service_remove(struct tb_service *svc);

/* Ring management functions */
void anarchy_ring_frame_callback(struct tb_ring *ring, struct ring_frame *frame, bool canceled);
int anarchy_ring_init(struct anarchy_device *adev, struct anarchy_ring *ring);
void anarchy_ring_cleanup(struct anarchy_device *adev, struct anarchy_ring *ring);
int anarchy_ring_start(struct anarchy_device *adev, struct anarchy_ring *ring, bool tx);
void anarchy_ring_stop(struct anarchy_device *adev, struct anarchy_ring *ring);

#endif /* _ANARCHY_THUNDERBOLT_H_ */ 