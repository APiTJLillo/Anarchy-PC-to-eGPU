#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy-PC-to-eGPU");
MODULE_DESCRIPTION("Compatibility layer for Anarchy-PC-to-eGPU");
MODULE_VERSION("1.0");
MODULE_INFO(intree, "Y");
MODULE_INFO(btf, "Y");

// Forward declarations for anarchy module functions
extern int anarchy_init(void);
extern void anarchy_exit(void);

static int __init compat_init(void)
{
    printk(KERN_INFO "Anarchy compatibility layer loaded\n");
    return anarchy_init();
}

static void __exit compat_exit(void)
{
    anarchy_exit();
    printk(KERN_INFO "Anarchy compatibility layer unloaded\n");
}

module_init(compat_init);
module_exit(compat_exit);
