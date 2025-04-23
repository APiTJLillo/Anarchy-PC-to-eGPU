#ifndef MOCK_THUNDERBOLT_H
#define MOCK_THUNDERBOLT_H

#include <stdbool.h>
#include <stddef.h>

/* Mock device structure for testing */
struct device {
    void *driver_data;
};

/* Mock Thunderbolt structures and defines for testing */
struct tb_cfg_request {
    size_t size;
    void *data;
};

struct tb_port {
    struct device dev;
    int port;
    bool enabled;
};

struct tb_switch {
    struct device dev;
    struct tb_port *ports;
    int num_ports;
};

struct tb {
    struct device dev;
    struct tb_switch *switch_list;
    int num_switches;
};

/* Mock functions */
static inline struct tb *tb_probe(void) { return NULL; }
static inline void tb_remove(struct tb *tb) { }
static inline int tb_register_protocol(void) { return 0; }
static inline void tb_unregister_protocol(void) { }

#endif /* MOCK_THUNDERBOLT_H */
