#ifndef ANARCHY_PCIE_H
#define ANARCHY_PCIE_H

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>

/* PCIe Configuration Space - must be defined before anarchy-device.h */
struct anarchy_pcie_caps {
    u8 pm_cap;
    u16 pm_ctrl;
    u8 msi_cap;
    u16 msi_ctrl;
    u64 msi_addr;
    u16 msi_data;
    u8 pcie_cap;
    u16 pcie_ctrl;
    u32 link_cap;
    u16 link_ctrl;
    u16 link_status;
    u32 dev_cap;    /* Device capabilities */
    u16 dev_ctrl;   /* Device control */
};

struct anarchy_pcie_config {
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8 revision;
    u8 header_type;
    u8 cache_line_size;
    u8 latency_timer;
    u8 bist;
    u32 bar[6];
    u32 bar_mask[6];
    u32 rom_base;
    bool rom_enabled;
    u8 interrupt_line;
    u8 interrupt_pin;
    void __iomem *mmio_base;
    void __iomem *mmio_base2;  /* Second MMIO region */
    void __iomem *fb_base;
    struct anarchy_pcie_caps caps;
};

#include "anarchy-device.h"


/* Function declarations */
int anarchy_pcie_init(struct pci_dev *pdev, struct anarchy_pcie_state *pcie);
void anarchy_pcie_exit(struct anarchy_pcie_state *pcie);
int anarchy_pcie_setup_bars(struct anarchy_pcie_state *pcie);
int anarchy_pcie_setup_capabilities(struct anarchy_pcie_state *pcie);
int anarchy_pcie_read_config(struct anarchy_pcie_state *pcie, int where, int size, u32 *val);
int anarchy_pcie_write_config(struct anarchy_pcie_state *pcie, int where, int size, u32 val);
void anarchy_pcie_check_link(struct anarchy_pcie_state *pcie);
void anarchy_pcie_retrain_link(struct anarchy_pcie_state *pcie);
int anarchy_pcie_train_link(struct anarchy_pcie_state *pcie);
void anarchy_pcie_disable(struct anarchy_pcie_state *pcie);
int anarchy_pcie_enable(struct anarchy_pcie_state *pcie);
void anarchy_pcie_handle_error(struct anarchy_pcie_state *pcie, enum anarchy_pcie_error_type error);
int anarchy_pcie_set_link_control(struct anarchy_pcie_state *pcie, u32 control_bits);

/* Constants */
#define ANARCHY_PCIE_RETRY_DELAY_MS 1000
#define ANARCHY_PCIE_TRAINING_TIMEOUT_MS 500

#endif /* ANARCHY_PCIE_H */
