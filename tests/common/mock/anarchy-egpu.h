#ifndef MOCK_ANARCHY_EGPU_H
#define MOCK_ANARCHY_EGPU_H

#include <stddef.h>
#include <stdbool.h>
#include "anarchy-device.h"

/* Mock eGPU structures and functions needed for testing */
struct anarchy_egpu {
    struct anarchy_device *dev;
    bool initialized;
};

/* Mock initialization functions */
int anarchy_egpu_init(void);
void anarchy_egpu_exit(void);

#endif /* MOCK_ANARCHY_EGPU_H */
