#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_MITIGATION_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

KSYMTAB_FUNC(anarchy_get_device, "_gpl", "");

SYMBOL_CRC(anarchy_get_device, 0x6ee1b62c, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\xe6\x0b\xca\xe7"
	"tb_ring_start\0\0\0"
	"\x18\x00\x00\x00\xed\x25\xcd\x49"
	"alloc_workqueue\0"
	"\x24\x00\x00\x00\x0e\xd7\x3a\x4a"
	"wait_for_completion_timeout\0"
	"\x18\x00\x00\x00\x57\x5e\xa1\xd4"
	"devm_kmalloc\0\0\0\0"
	"\x1c\x00\x00\x00\x19\x3b\xce\xc3"
	"__tb_ring_enqueue\0\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\xb5\x41\x87\x60"
	"__init_swait_queue_head\0"
	"\x18\x00\x00\x00\xf6\x9c\x99\xc4"
	"tb_ring_free\0\0\0\0"
	"\x10\x00\x00\x00\x53\x39\xc0\xed"
	"iounmap\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x1c\x00\x00\x00\xe9\x2e\xd4\x06"
	"tb_ring_alloc_tx\0\0\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x20\x00\x00\x00\x0b\x05\xdb\x34"
	"_raw_spin_lock_irqsave\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\xd8\x7e\x99\x92"
	"_printk\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x20\x00\x00\x00\xe9\x0d\x1d\x44"
	"__kmalloc_large_noprof\0\0"
	"\x14\x00\x00\x00\xed\x5e\x1f\xcc"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x14\x00\x00\x00\xa4\x32\x5e\x63"
	"devm_kfree\0\0"
	"\x14\x00\x00\x00\xd6\xe0\x4e\x41"
	"_dev_err\0\0\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x1c\x00\x00\x00\x0c\xd2\x03\x8c"
	"destroy_workqueue\0\0\0"
	"\x18\x00\x00\x00\x2e\xcd\xf9\x2a"
	"dma_alloc_attrs\0"
	"\x10\x00\x00\x00\x09\xcd\x80\xde"
	"ioremap\0"
	"\x10\x00\x00\x00\xa7\xd0\x9a\x44"
	"memcmp\0\0"
	"\x24\x00\x00\x00\x88\xdd\x1d\x25"
	"tb_register_service_driver\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x14\x00\x00\x00\x4b\x94\x99\xed"
	"_dev_warn\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\x54\xea\xa5\xd9"
	"__init_waitqueue_head\0\0\0"
	"\x18\x00\x00\x00\xd1\xed\x36\x0a"
	"dma_free_attrs\0\0"
	"\x28\x00\x00\x00\xe5\xc5\x99\xc6"
	"tb_unregister_service_driver\0\0\0\0"
	"\x20\x00\x00\x00\xee\xc4\x9e\x76"
	"__kmalloc_cache_noprof\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x1c\x00\x00\x00\x40\x6b\x94\x84"
	"tb_ring_alloc_rx\0\0\0\0"
	"\x18\x00\x00\x00\x31\x70\x2c\x1a"
	"tb_ring_stop\0\0\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x18\x00\x00\x00\x32\xa5\x35\xfe"
	"kmalloc_caches\0\0"
	"\x14\x00\x00\x00\xd3\x85\x33\x2d"
	"system_wq\0\0\0"
	"\x18\x00\x00\x00\xfd\x1a\x4a\xa7"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "thunderbolt");


MODULE_INFO(srcversion, "F3C31A22E493AE8F5137E86");
