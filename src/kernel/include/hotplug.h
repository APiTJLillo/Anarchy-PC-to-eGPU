#ifndef ANARCHY_HOTPLUG_H
#define ANARCHY_HOTPLUG_H

#include <linux/thunderbolt.h>
#include "anarchy_device.h"
#include "pcie_types.h"

/* Hotplug initialization */
int anarchy_hotplug_init(struct anarchy_device *adev);
void anarchy_hotplug_exit(struct anarchy_device *adev);

/* Device hotplug callbacks */
void handle_device_connect(struct anarchy_device *adev);
void handle_device_disconnect(struct anarchy_device *adev);

/* PCIe link management */
int anarchy_pcie_enable_link(struct anarchy_device *adev);
void anarchy_pcie_disable_link(struct anarchy_device *adev);
int anarchy_pcie_optimize_settings(struct anarchy_device *adev);

/* Game optimization */
int anarchy_optimize_for_game(struct anarchy_device *adev, const char *game_profile);

#endif /* ANARCHY_HOTPLUG_H */
