#ifndef _ANARCHY_PERF_H_
#define _ANARCHY_PERF_H_

#include <linux/types.h>
#include "../../include/anarchy-egpu.h"

/* Event types for performance monitoring */
enum anarchy_tb_event {
    TB_EVENT_CONNECT,
    TB_EVENT_DISCONNECT,
    TB_EVENT_LINK_ERROR,
    TB_EVENT_TRAINING_START,
    TB_EVENT_TRAINING_COMPLETE,
    TB_EVENT_RECOVERY_START,
    TB_EVENT_RECOVERY_COMPLETE,
    TB_EVENT_MAX
};

/* Performance monitoring functions */
void anarchy_perf_init(void);
void anarchy_perf_cleanup(void);
void anarchy_perf_record_event(enum anarchy_tb_event event, u64 timestamp);
void anarchy_perf_dump_stats(void);
void anarchy_perf_tb_event(enum anarchy_tb_event event, int value);

#endif /* _ANARCHY_PERF_H_ */
