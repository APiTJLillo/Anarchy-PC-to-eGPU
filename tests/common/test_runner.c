#include "test_framework.h"

/* Global test registry */
struct test_function g_tests[MAX_TESTS];
int g_test_count = 0;

/* Test runner implementation */
void run_all_tests(void) {
    printf("Running %d tests...\n", g_test_count);
    int passed = 0;

    for (int i = 0; i < g_test_count; i++) {
        printf("\nRunning test: %s\n", g_tests[i].name);
        before_each();
        g_tests[i].func();
        after_each();
        passed++;
        printf("Test %s: PASSED\n", g_tests[i].name);
    }

    printf("\nTest summary: %d/%d tests passed\n", passed, g_test_count);
}
