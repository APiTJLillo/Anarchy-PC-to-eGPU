#include <linux/module.h>
#include <linux/kernel.h>
#include "anarchy-perf.h"

void anarchy_perf_tb_event(enum anarchy_tb_event event, int value)
{
    /* TODO: Implement performance monitoring */
}

EXPORT_SYMBOL_GPL(anarchy_perf_tb_event);

/* Module information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy eGPU Team");
MODULE_DESCRIPTION("Performance Monitoring for eGPU Driver");
