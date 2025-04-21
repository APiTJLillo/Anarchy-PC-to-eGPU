#ifndef _ANARCHY_TEST_H_
#define _ANARCHY_TEST_H_

#include <linux/types.h>
#include <linux/completion.h>
#include "anarchy-egpu.h"

/* Test categories */
#define ANARCHY_TEST_DMA          BIT(0)
#define ANARCHY_TEST_PCIE         BIT(1)
#define ANARCHY_TEST_THUNDERBOLT  BIT(2)
#define ANARCHY_TEST_STRESS       BIT(3)
#define ANARCHY_TEST_PERFORMANCE  BIT(4)

/* Error injection points */
#define ANARCHY_ERR_TB_CONNECT    BIT(0)  /* Thunderbolt connection failures */
#define ANARCHY_ERR_TB_TRANSFER   BIT(1)  /* Thunderbolt transfer errors */
#define ANARCHY_ERR_PCIE_CONFIG   BIT(2)  /* PCIe configuration errors */
#define ANARCHY_ERR_DMA_ALLOC     BIT(3)  /* DMA allocation failures */
#define ANARCHY_ERR_DMA_TRANSFER  BIT(4)  /* DMA transfer errors */
#define ANARCHY_ERR_MEMORY        BIT(5)  /* Memory allocation failures */

/* Test result structure */
struct anarchy_test_result {
    const char *name;         /* Test name */
    bool passed;             /* Test result */
    char *message;           /* Result message */
    u64 duration_ns;         /* Test duration in nanoseconds */
};

/* Test case structure */
struct anarchy_test_case {
    const char *name;        /* Test name */
    unsigned int categories; /* Test category */
    int (*run)(struct anarchy_device *dev, struct anarchy_test_result *result);
    bool enabled;           /* Whether test is enabled */
};

/* Error injection configuration */
struct anarchy_error_config {
    u32 error_mask;         /* Which errors to inject */
    u32 error_frequency;    /* How often to inject errors (1 in N operations) */
    bool enabled;           /* Whether error injection is enabled */
};

/* Test statistics */
struct anarchy_test_stats {
    atomic_t total_tests;
    atomic_t passed_tests;
    atomic_t failed_tests;
    atomic_t skipped_tests;
    atomic64_t total_duration;
};

/* Function declarations */
int anarchy_test_init(void);
void anarchy_test_exit(void);
int anarchy_run_tests(struct anarchy_device *dev, unsigned int categories);
void anarchy_test_stats_dump(void);

/* Error injection control */
void anarchy_error_init(void);
void anarchy_error_enable(u32 error_mask);
void anarchy_error_disable(u32 error_mask);
bool anarchy_should_inject_error(u32 error_type);

/* Test case registration */
#define ANARCHY_TEST_CASE(_name, _categories, _func) \
    { .name = #_name, .categories = _categories, .run = _func }

/* External test cases */
extern struct anarchy_test_case anarchy_dma_tests[];
extern struct anarchy_test_case anarchy_pcie_tests[];
extern struct anarchy_test_case anarchy_tb_tests[];
extern struct anarchy_test_case anarchy_stress_tests[];

/* Global test configuration */
extern struct anarchy_error_config anarchy_errors;
extern struct anarchy_test_stats anarchy_stats;

#endif /* _ANARCHY_TEST_H_ */ 