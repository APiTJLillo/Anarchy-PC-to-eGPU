#ifndef ANARCHY_SERVICE_PROBE_H
#define ANARCHY_SERVICE_PROBE_H

#include <linux/device.h>
#include <linux/thunderbolt.h>
#include "anarchy_device.h"

/* Performance monitoring function declarations */
int init_performance_monitoring(struct anarchy_device *adev);
void cleanup_performance_monitoring(struct anarchy_device *adev);

/* Declare module parameters */
extern int power_limit;
extern int num_dma_channels;

/* Service probe declarations */
int anarchy_service_probe(struct tb_service *svc, const struct tb_service_id *id);
void anarchy_service_remove(struct tb_service *svc);

#endif /* ANARCHY_SERVICE_PROBE_H */
