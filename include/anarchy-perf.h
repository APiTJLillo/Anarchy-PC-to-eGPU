#ifndef _ANARCHY_PERF_H_
#define _ANARCHY_PERF_H_

#include <linux/types.h>

/* Thunderbolt event types */
enum anarchy_tb_event {
    ANARCHY_TB_CONNECT,
    ANARCHY_TB_DISCONNECT,
    ANARCHY_TB_ERROR
};

/* Performance monitoring functions */
int anarchy_perf_init(struct dentry *parent_dir);
void anarchy_perf_exit(void);
void anarchy_perf_reset(void);

/* DMA performance tracking */
void anarchy_perf_dma_transfer(size_t bytes, bool is_tx, s64 latency_ns);
void anarchy_perf_dma_error(void);

/* Thunderbolt performance tracking */
void anarchy_perf_tb_event(enum anarchy_tb_event event, s64 latency_ns);

#endif /* _ANARCHY_PERF_H_ */ 