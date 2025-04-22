#ifndef ANARCHY_THUNDERBOLT_H
#define ANARCHY_THUNDERBOLT_H

#include <linux/thunderbolt.h>
#include "forward.h"
#include "anarchy_device.h"

/* Thunderbolt service protocol configuration */
#define TBSVC_MATCH_PROTOCOL_KEY       BIT(0)
#define TBSVC_MATCH_PROTOCOL_ID        BIT(1)
#define TBSVC_MATCH_PROTOCOL_VERSION   BIT(2)

#define ANARCHY_SERVICE_MATCH_FLAGS (TBSVC_MATCH_PROTOCOL_KEY | \
                                   TBSVC_MATCH_PROTOCOL_ID | \
                                   TBSVC_MATCH_PROTOCOL_VERSION)

#define ANARCHY_SERVICE_PROTOCOL_KEY    0x4E574F44  /* "DOWN" */
#define ANARCHY_SERVICE_PROTOCOL_ID     0x01
#define ANARCHY_PROTOCOL_VERSION        0x01

/* Service configuration */
struct tb_service_config {
    u32 max_lanes;
    u32 max_speed;
    u32 power_limit;
    bool endpoint_mode;
};

/* Core thunderbolt interface */
int anarchy_tb_init(void);
void anarchy_tb_exit(void);
void anarchy_tb_pcie_disable(struct anarchy_device *adev);
struct anarchy_device *anarchy_get_device(void);

/* Error handling */
void anarchy_handle_connection_error(struct anarchy_device *adev, int err);
int anarchy_tb_reset_connection(struct anarchy_device *adev);
void anarchy_tb_error_recovery(struct work_struct *work);

/* Service configuration */
int tb_service_configure(struct tb_service *svc, 
                        const struct tb_service_config *config);
int tb_service_start(struct tb_service *svc);
void tb_service_stop(struct tb_service *svc);
int tb_service_reset(struct tb_service *svc);

#endif /* ANARCHY_THUNDERBOLT_H */
