#ifndef _ANARCHY_PERF_H_
#define _ANARCHY_PERF_H_

#include <linux/types.h>
#include "../../include/anarchy-egpu.h"

/* Performance monitoring functions */
void anarchy_perf_init(void);
void anarchy_perf_cleanup(void);
void anarchy_perf_record_event(enum anarchy_tb_event event, u64 timestamp);
void anarchy_perf_dump_stats(void);

#endif /* _ANARCHY_PERF_H_ */ 