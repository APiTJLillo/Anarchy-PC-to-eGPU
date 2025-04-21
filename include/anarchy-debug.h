#ifndef _ANARCHY_DEBUG_H_
#define _ANARCHY_DEBUG_H_

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/ktime.h>

/* Debug levels */
#define ANARCHY_DEBUG_NONE    0
#define ANARCHY_DEBUG_ERROR   1
#define ANARCHY_DEBUG_WARN    2
#define ANARCHY_DEBUG_INFO    3
#define ANARCHY_DEBUG_DEBUG   4
#define ANARCHY_DEBUG_TRACE   5

/* Debug categories */
#define ANARCHY_CAT_INIT     BIT(0)  /* Initialization/cleanup */
#define ANARCHY_CAT_TB       BIT(1)  /* Thunderbolt operations */
#define ANARCHY_CAT_PCIE     BIT(2)  /* PCIe operations */
#define ANARCHY_CAT_DMA      BIT(3)  /* DMA operations */
#define ANARCHY_CAT_IRQ      BIT(4)  /* Interrupt handling */
#define ANARCHY_CAT_MEM      BIT(5)  /* Memory management */
#define ANARCHY_CAT_PERF     BIT(6)  /* Performance metrics */

/* Performance counters */
struct anarchy_perf_counters {
    /* Transfer statistics */
    atomic64_t bytes_tx;
    atomic64_t bytes_rx;
    atomic_t transfers_completed;
    atomic_t transfers_failed;
    
    /* Latency measurements (in nanoseconds) */
    ktime_t last_tx_time;
    ktime_t last_rx_time;
    atomic64_t total_tx_time;
    atomic64_t total_rx_time;
    atomic_t tx_count;
    atomic_t rx_count;
    
    /* PCIe statistics */
    atomic_t config_reads;
    atomic_t config_writes;
    atomic_t mmio_reads;
    atomic_t mmio_writes;
    
    /* Error counters */
    atomic_t tb_errors;
    atomic_t pcie_errors;
    atomic_t dma_errors;
};

/* Debug configuration */
struct anarchy_debug_config {
    u32 debug_level;
    u32 debug_categories;
    bool perf_enabled;
    struct anarchy_perf_counters counters;
};

/* Debug macros */
#define anarchy_dbg(level, cat, fmt, ...) \
    do { \
        if (level <= anarchy_debug.debug_level && \
            (cat & anarchy_debug.debug_categories)) \
            pr_info("Anarchy eGPU: " fmt, ##__VA_ARGS__); \
    } while (0)

#define anarchy_err(fmt, ...) \
    anarchy_dbg(ANARCHY_DEBUG_ERROR, ANARCHY_CAT_INIT, fmt, ##__VA_ARGS__)

#define anarchy_warn(fmt, ...) \
    anarchy_dbg(ANARCHY_DEBUG_WARN, ANARCHY_CAT_INIT, fmt, ##__VA_ARGS__)

#define anarchy_info(fmt, ...) \
    anarchy_dbg(ANARCHY_DEBUG_INFO, ANARCHY_CAT_INIT, fmt, ##__VA_ARGS__)

/* Performance measurement macros */
#define ANARCHY_PERF_START(counter) \
    ktime_t start_##counter = ktime_get()

#define ANARCHY_PERF_END(counter) \
    do { \
        if (anarchy_debug.perf_enabled) { \
            ktime_t end = ktime_get(); \
            atomic64_add(ktime_to_ns(ktime_sub(end, start_##counter)), \
                        &anarchy_debug.counters.total_##counter##_time); \
            atomic_inc(&anarchy_debug.counters.counter##_count); \
        } \
    } while (0)

/* Function declarations */
void anarchy_debug_init(void);
void anarchy_debug_exit(void);
void anarchy_perf_reset(void);
void anarchy_perf_dump(void);

/* Global debug configuration */
extern struct anarchy_debug_config anarchy_debug;

#endif /* _ANARCHY_DEBUG_H_ */ 