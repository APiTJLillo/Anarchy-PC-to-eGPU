#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include "../include/anarchy-egpu.h"

static int __init anarchy_init(void)
{
	int ret;

	pr_info("Initializing Anarchy eGPU driver\n");

	/* Initialize Thunderbolt */
	ret = anarchy_tb_init();
	if (ret) {
		pr_err("Failed to initialize Thunderbolt: %d\n", ret);
		return ret;
	}

	return 0;
}

static void __exit anarchy_exit(void)
{
	pr_info("Exiting Anarchy eGPU driver\n");
	anarchy_tb_exit();
}

module_init(anarchy_init);
module_exit(anarchy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Anarchy eGPU Driver"); 