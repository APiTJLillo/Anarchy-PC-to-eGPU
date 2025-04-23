#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/thunderbolt.h>
#include "include/anarchy_device.h"
#include "include/pcie_types.h"
#include "include/service_probe.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy-PC-to-eGPU");
MODULE_DESCRIPTION("Anarchy PC to eGPU Driver");
MODULE_VERSION("1.0");
MODULE_INFO(intree, "Y");
MODULE_INFO(btf, "Y");

// Allow BTF mismatch by default
bool allow_btf_mismatch = true;
module_param(allow_btf_mismatch, bool, 0644);
MODULE_PARM_DESC(allow_btf_mismatch, "Allow BTF mismatch (default: true)");

// Define a simple service table
static const struct tb_service_id anarchy_service_table[] = {
    {
        .match_flags = ANARCHY_SERVICE_MATCH_PROTOCOL,
        .protocol_key = 0x42,  /* Match ANARCHY_PROTOCOL_KEY */
    },
    { }
};

// Simple init function
static int __init anarchy_init(void)
{
    printk(KERN_INFO "Anarchy PC to eGPU Driver loaded\n");
    return 0;
}

// Simple exit function
static void __exit anarchy_exit(void)
{
    printk(KERN_INFO "Anarchy PC to eGPU Driver unloaded\n");
}

module_init(anarchy_init);
module_exit(anarchy_exit);
