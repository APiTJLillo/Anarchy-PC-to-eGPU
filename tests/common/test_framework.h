#ifndef ANARCHY_TEST_FRAMEWORK_H
#define ANARCHY_TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "anarchy-device.h"

/* Forward declarations */
struct anarchy_device;

/* Test case macros */
#define CONCATENATE(x, y) x##y
#define MAKE_TEST_NAME(prefix, line) CONCATENATE(prefix, line)

#define TEST_CASE(name) void MAKE_TEST_NAME(test_case_, __LINE__)(void)
#define TEST(name) void MAKE_TEST_NAME(test_, __LINE__)(void)
#define SECTION(name) do

/* Array size macro */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Assert macros */
#define REQUIRE(condition) \
    do { \
        if (!(condition)) { \
            printf("Test failed: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

/* Test function registration */
struct test_function {
    const char *name;
    void (*func)(void);
};

#define MAX_TESTS 100
extern struct test_function g_tests[MAX_TESTS];
extern int g_test_count;

#define REGISTER_TEST(func_name, test_func) \
    __attribute__((constructor)) static void register_##func_name(void) { \
        if (g_test_count < MAX_TESTS) { \
            g_tests[g_test_count].name = #func_name; \
            g_tests[g_test_count].func = test_func; \
            g_test_count++; \
        } \
    }

#define BEFORE_EACH() void before_each(void)
#define AFTER_EACH() void after_each(void)

/* Test context structure */
struct test_context {
    void *data;
    int status;
    char *error_message;
};

/* Test utilities */
struct test_context *create_test_context(void);
void free_test_context(struct test_context *ctx);

/* Error injection utilities */
struct error_injection_params {
    int type;
    int delay;
};

#define ERR_DMA_TIMEOUT     1
#define ERR_COMMAND_TIMEOUT 2
#define ERR_GPU_FAULT       3
#define ERR_MEMORY_ERROR    4

void inject_error(struct anarchy_device *dev, struct error_injection_params *params);

/* State checking utilities */
#define COMP_STATE_STOPPED      0
#define TRANSFER_STATE_STOPPED  0
#define CMD_STATE_STOPPED       0

int check_gpu_computation_state(struct anarchy_device *dev);
int check_memory_transfer_state(struct anarchy_device *dev);
int check_command_processor_state(struct anarchy_device *dev);
int check_resource_leaks(struct anarchy_device *dev);

/* Test data generation */
char *generate_test_data(size_t size);

/* Test hooks - these must be implemented by the test file */
extern void before_each(void);
extern void after_each(void);

/* Test runner */
void run_all_tests(void);

#endif /* ANARCHY_TEST_FRAMEWORK_H */
