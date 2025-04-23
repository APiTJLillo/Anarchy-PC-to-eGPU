#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdint.h>

/* Sleep utilities */
void msleep(unsigned int msecs);

/* Random number generation */
uint32_t get_random_u32(void);

#endif /* TEST_UTILS_H */
