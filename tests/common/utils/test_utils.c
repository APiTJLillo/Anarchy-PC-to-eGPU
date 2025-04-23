#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "test_utils.h"

void msleep(unsigned int msecs) {
    usleep(msecs * 1000);
}

uint32_t get_random_u32(void) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    return (uint32_t)rand();
}
