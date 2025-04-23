#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/percpu.h>

#include "anarchy-egpu.h"
#include "anarchy-debug.h"

/* Per-CPU performance counters */
struct anarchy_perf_counters {
    /* DMA statistics */
    atomic64_t dma_bytes_tx;
    atomic64_t dma_bytes_rx;
    atomic_t dma_ops_tx;
    atomic_t dma_ops_rx;
    atomic_t dma_errors;

    /* Latency tracking (in nanoseconds) */
    atomic64_t dma_latency_total;
    atomic_t dma_latency_samples;
    atomic64_t dma_latency_max;
    atomic64_t dma_latency_min;

    /* Thunderbolt statistics */
    atomic_t tb_connects;
    atomic_t tb_disconnects;
    atomic_t tb_errors;
    atomic64_t tb_connect_latency_total;
    atomic_t tb_connect_samples;
};

/* Global performance data */
struct anarchy_perf_data {
    struct anarchy_perf_counters __percpu *counters;
    ktime_t start_time;
    struct dentry *debugfs_dir;
};

static struct anarchy_perf_data perf_data;

/**
 * anarchy_perf_reset - Reset all performance counters
 */
void anarchy_perf_reset(void)
{
    int cpu;

    for_each_possible_cpu(cpu) {
        struct anarchy_perf_counters *counters = per_cpu_ptr(perf_data.counters, cpu);

        /* Reset DMA counters */
        atomic64_set(&counters->dma_bytes_tx, 0);
        atomic64_set(&counters->dma_bytes_rx, 0);
        atomic_set(&counters->dma_ops_tx, 0);
        atomic_set(&counters->dma_ops_rx, 0);
        atomic_set(&counters->dma_errors, 0);

        /* Reset latency tracking */
        atomic64_set(&counters->dma_latency_total, 0);
        atomic_set(&counters->dma_latency_samples, 0);
        atomic64_set(&counters->dma_latency_max, 0);
        atomic64_set(&counters->dma_latency_min, S64_MAX);

        /* Reset Thunderbolt counters */
        atomic_set(&counters->tb_connects, 0);
        atomic_set(&counters->tb_disconnects, 0);
        atomic_set(&counters->tb_errors, 0);
        atomic64_set(&counters->tb_connect_latency_total, 0);
        atomic_set(&counters->tb_connect_samples, 0);
    }

    perf_data.start_time = ktime_get();
}

/**
 * anarchy_perf_dma_transfer - Record DMA transfer statistics
 */
void anarchy_perf_dma_transfer(size_t bytes, bool is_tx, s64 latency_ns)
{
    struct anarchy_perf_counters *counters;

    counters = this_cpu_ptr(perf_data.counters);

    if (is_tx) {
        atomic64_add(bytes, &counters->dma_bytes_tx);
        atomic_inc(&counters->dma_ops_tx);
    } else {
        atomic64_add(bytes, &counters->dma_bytes_rx);
        atomic_inc(&counters->dma_ops_rx);
    }

    /* Update latency statistics */
    atomic64_add(latency_ns, &counters->dma_latency_total);
    atomic_inc(&counters->dma_latency_samples);
    atomic64_max(&counters->dma_latency_max, latency_ns);
    atomic64_min(&counters->dma_latency_min, latency_ns);
}

/**
 * anarchy_perf_dma_error - Record DMA error
 */
void anarchy_perf_dma_error(void)
{
    struct anarchy_perf_counters *counters = this_cpu_ptr(perf_data.counters);
    atomic_inc(&counters->dma_errors);
}

/**
 * anarchy_perf_tb_event - Record Thunderbolt event
 */
void anarchy_perf_tb_event(enum anarchy_tb_event event, s64 latency_ns)
{
    struct anarchy_perf_counters *counters = this_cpu_ptr(perf_data.counters);

    switch (event) {
    case ANARCHY_TB_CONNECT:
        atomic_inc(&counters->tb_connects);
        atomic64_add(latency_ns, &counters->tb_connect_latency_total);
        atomic_inc(&counters->tb_connect_samples);
        break;
    case ANARCHY_TB_DISCONNECT:
        atomic_inc(&counters->tb_disconnects);
        break;
    case ANARCHY_TB_ERROR:
        atomic_inc(&counters->tb_errors);
        break;
    }
}

/* Debugfs file operations */

static void *perf_seq_start(struct seq_file *s, loff_t *pos)
{
    return *pos ? NULL : SEQ_START_TOKEN;
}

static void *perf_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    ++*pos;
    return NULL;
}

static void perf_seq_stop(struct seq_file *s, void *v) { }

static int perf_seq_show(struct seq_file *s, void *v)
{
    s64 total_bytes_tx = 0, total_bytes_rx = 0;
    s64 total_ops_tx = 0, total_ops_rx = 0;
    s64 total_errors = 0;
    s64 total_latency = 0, total_samples = 0;
    s64 max_latency = 0, min_latency = S64_MAX;
    s64 total_connects = 0, total_disconnects = 0;
    s64 total_tb_errors = 0;
    s64 total_tb_latency = 0, total_tb_samples = 0;
    s64 runtime_ms;
    int cpu;

    /* Aggregate counters from all CPUs */
    for_each_possible_cpu(cpu) {
        struct anarchy_perf_counters *counters = per_cpu_ptr(perf_data.counters, cpu);

        total_bytes_tx += atomic64_read(&counters->dma_bytes_tx);
        total_bytes_rx += atomic64_read(&counters->dma_bytes_rx);
        total_ops_tx += atomic_read(&counters->dma_ops_tx);
        total_ops_rx += atomic_read(&counters->dma_ops_rx);
        total_errors += atomic_read(&counters->dma_errors);

        total_latency += atomic64_read(&counters->dma_latency_total);
        total_samples += atomic_read(&counters->dma_latency_samples);
        max_latency = max(max_latency,
                         atomic64_read(&counters->dma_latency_max));
        min_latency = min(min_latency,
                         atomic64_read(&counters->dma_latency_min));

        total_connects += atomic_read(&counters->tb_connects);
        total_disconnects += atomic_read(&counters->tb_disconnects);
        total_tb_errors += atomic_read(&counters->tb_errors);
        total_tb_latency += atomic64_read(&counters->tb_connect_latency_total);
        total_tb_samples += atomic_read(&counters->tb_connect_samples);
    }

    runtime_ms = ktime_to_ms(ktime_sub(ktime_get(), perf_data.start_time));

    /* Print statistics */
    seq_puts(s, "Anarchy eGPU Performance Statistics\n");
    seq_puts(s, "================================\n\n");

    seq_printf(s, "Runtime: %lld.%03lld seconds\n\n",
               runtime_ms / 1000, runtime_ms % 1000);

    seq_puts(s, "DMA Statistics:\n");
    seq_printf(s, "  TX: %lld bytes in %lld operations (%.2f MB/s)\n",
               total_bytes_tx, total_ops_tx,
               (float)(total_bytes_tx * 1000) / (runtime_ms * 1024 * 1024));
    seq_printf(s, "  RX: %lld bytes in %lld operations (%.2f MB/s)\n",
               total_bytes_rx, total_ops_rx,
               (float)(total_bytes_rx * 1000) / (runtime_ms * 1024 * 1024));
    seq_printf(s, "  Errors: %lld\n\n", total_errors);

    if (total_samples > 0) {
        seq_puts(s, "DMA Latency:\n");
        seq_printf(s, "  Average: %lld ns\n",
                   total_latency / total_samples);
        seq_printf(s, "  Maximum: %lld ns\n", max_latency);
        seq_printf(s, "  Minimum: %lld ns\n\n",
                   min_latency == S64_MAX ? 0 : min_latency);
    }

    seq_puts(s, "Thunderbolt Statistics:\n");
    seq_printf(s, "  Connections: %lld\n", total_connects);
    seq_printf(s, "  Disconnections: %lld\n", total_disconnects);
    seq_printf(s, "  Errors: %lld\n", total_tb_errors);
    if (total_tb_samples > 0) {
        seq_printf(s, "  Average Connect Time: %lld ns\n",
                   total_tb_latency / total_tb_samples);
    }

    return 0;
}

static const struct seq_operations perf_seq_ops = {
    .start = perf_seq_start,
    .next  = perf_seq_next,
    .stop  = perf_seq_stop,
    .show  = perf_seq_show
};

static int perf_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &perf_seq_ops);
}

static const struct file_operations perf_fops = {
    .owner   = THIS_MODULE,
    .open    = perf_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release
};

/**
 * anarchy_perf_init - Initialize performance monitoring
 */
int anarchy_perf_init(struct dentry *parent_dir)
{
    /* Allocate per-CPU counters */
    perf_data.counters = alloc_percpu(struct anarchy_perf_counters);
    if (!perf_data.counters)
        return -ENOMEM;

    /* Create debugfs entries */
    if (parent_dir) {
        perf_data.debugfs_dir = debugfs_create_dir("performance", parent_dir);
        if (perf_data.debugfs_dir) {
            debugfs_create_file("statistics", 0444, perf_data.debugfs_dir,
                              NULL, &perf_fops);
        }
    }

    /* Initialize counters */
    anarchy_perf_reset();

    return 0;
}

/**
 * anarchy_perf_exit - Clean up performance monitoring
 */
void anarchy_perf_exit(void)
{
    debugfs_remove_recursive(perf_data.debugfs_dir);
    free_percpu(perf_data.counters);
} 