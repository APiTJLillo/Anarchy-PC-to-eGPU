#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy-PC-to-eGPU");
MODULE_DESCRIPTION("Minimal Anarchy PC to eGPU Driver");
MODULE_VERSION("1.0");
MODULE_INFO(intree, "Y");
MODULE_INFO(btf, "Y");

// Allow BTF mismatch by default
bool allow_btf_mismatch = true;
module_param(allow_btf_mismatch, bool, 0644);
MODULE_PARM_DESC(allow_btf_mismatch, "Allow BTF mismatch (default: true)");

// Simple init function
static int __init anarchy_minimal_init(void)
{
    printk(KERN_INFO "Anarchy PC to eGPU Minimal Driver loaded\n");
    return 0;
}

// Simple exit function
static void __exit anarchy_minimal_exit(void)
{
    printk(KERN_INFO "Anarchy PC to eGPU Minimal Driver unloaded\n");
}

module_init(anarchy_minimal_init);
module_exit(anarchy_minimal_exit);
