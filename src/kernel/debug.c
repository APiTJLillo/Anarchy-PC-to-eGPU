#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/atomic.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"

/* Global debug configuration */
struct anarchy_debug_config anarchy_debug = {
    .debug_level = ANARCHY_DEBUG_INFO,
    .debug_categories = ANARCHY_CAT_INIT | ANARCHY_CAT_TB | 
                       ANARCHY_CAT_PCIE | ANARCHY_CAT_DMA,
    .perf_enabled = true,
};

/**
 * anarchy_debug_init - Initialize debug subsystem
 *
 * Sets up debug configuration and resets performance counters.
 */
void anarchy_debug_init(void)
{
    /* Reset all performance counters */
    anarchy_perf_reset();
    
    pr_info("Anarchy eGPU: Debug subsystem initialized (level=%d, categories=0x%x)\n",
            anarchy_debug.debug_level, anarchy_debug.debug_categories);
}

/**
 * anarchy_debug_exit - Clean up debug subsystem
 *
 * Dumps final performance statistics and cleans up resources.
 */
void anarchy_debug_exit(void)
{
    if (anarchy_debug.perf_enabled)
        anarchy_perf_dump();
        
    pr_info("Anarchy eGPU: Debug subsystem cleanup complete\n");
}

/**
 * anarchy_perf_reset - Reset all performance counters
 */
void anarchy_perf_reset(void)
{
    struct anarchy_perf_counters *counters = &anarchy_debug.counters;
    
    /* Reset transfer statistics */
    atomic64_set(&counters->bytes_tx, 0);
    atomic64_set(&counters->bytes_rx, 0);
    atomic_set(&counters->transfers_completed, 0);
    atomic_set(&counters->transfers_failed, 0);
    
    /* Reset latency measurements */
    counters->last_tx_time = ktime_set(0, 0);
    counters->last_rx_time = ktime_set(0, 0);
    atomic64_set(&counters->total_tx_time, 0);
    atomic64_set(&counters->total_rx_time, 0);
    atomic_set(&counters->tx_count, 0);
    atomic_set(&counters->rx_count, 0);
    
    /* Reset PCIe statistics */
    atomic_set(&counters->config_reads, 0);
    atomic_set(&counters->config_writes, 0);
    atomic_set(&counters->mmio_reads, 0);
    atomic_set(&counters->mmio_writes, 0);
    
    /* Reset error counters */
    atomic_set(&counters->tb_errors, 0);
    atomic_set(&counters->pcie_errors, 0);
    atomic_set(&counters->dma_errors, 0);
}

/**
 * anarchy_perf_dump - Dump performance statistics
 *
 * Prints current performance statistics to kernel log.
 */
void anarchy_perf_dump(void)
{
    struct anarchy_perf_counters *c = &anarchy_debug.counters;
    u64 tx_time, rx_time;
    u32 tx_count, rx_count;
    
    /* Calculate average latencies */
    tx_count = atomic_read(&c->tx_count);
    rx_count = atomic_read(&c->rx_count);
    tx_time = atomic64_read(&c->total_tx_time);
    rx_time = atomic64_read(&c->total_rx_time);
    
    pr_info("Anarchy eGPU: Performance Statistics:\n");
    pr_info("  Transfers:\n");
    pr_info("    Completed: %d, Failed: %d\n",
            atomic_read(&c->transfers_completed),
            atomic_read(&c->transfers_failed));
    pr_info("    TX: %lld bytes, Avg latency: %lld ns\n",
            atomic64_read(&c->bytes_tx),
            tx_count ? tx_time / tx_count : 0);
    pr_info("    RX: %lld bytes, Avg latency: %lld ns\n",
            atomic64_read(&c->bytes_rx),
            rx_count ? rx_time / rx_count : 0);
            
    pr_info("  PCIe Operations:\n");
    pr_info("    Config: %d reads, %d writes\n",
            atomic_read(&c->config_reads),
            atomic_read(&c->config_writes));
    pr_info("    MMIO: %d reads, %d writes\n",
            atomic_read(&c->mmio_reads),
            atomic_read(&c->mmio_writes));
            
    pr_info("  Errors:\n");
    pr_info("    Thunderbolt: %d\n", atomic_read(&c->tb_errors));
    pr_info("    PCIe: %d\n", atomic_read(&c->pcie_errors));
    pr_info("    DMA: %d\n", atomic_read(&c->dma_errors));
}

/* Module parameters for debug configuration */
module_param_named(debug_level, anarchy_debug.debug_level, uint, 0644);
MODULE_PARM_DESC(debug_level, "Debug verbosity level (0-5)");

module_param_named(debug_categories, anarchy_debug.debug_categories, uint, 0644);
MODULE_PARM_DESC(debug_categories, "Debug category mask (bitfield)");

module_param_named(perf_enabled, anarchy_debug.perf_enabled, bool, 0644);
MODULE_PARM_DESC(perf_enabled, "Enable performance monitoring (0/1)"); 