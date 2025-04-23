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

KSYMTAB_DATA(anarchy_driver, "_gpl", "");
KSYMTAB_DATA(anarchy_service_driver, "_gpl", "");
KSYMTAB_FUNC(tb_read32, "_gpl", "");
KSYMTAB_FUNC(tb_write32, "_gpl", "");
KSYMTAB_FUNC(anarchy_tb_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_tb_fini, "_gpl", "");
KSYMTAB_FUNC(anarchy_thunderbolt_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_thunderbolt_cleanup, "_gpl", "");
KSYMTAB_DATA(tb_bus_type, "_gpl", "");
KSYMTAB_FUNC(tb_service_driver_register, "_gpl", "");
KSYMTAB_FUNC(tb_service_driver_unregister, "_gpl", "");
KSYMTAB_FUNC(tb_service_register_handler, "_gpl", "");
KSYMTAB_FUNC(tb_service_unregister_handler, "_gpl", "");
KSYMTAB_FUNC(tb_service_reset_controller, "_gpl", "");
KSYMTAB_FUNC(tb_service_power_cycle_controller, "_gpl", "");
KSYMTAB_FUNC(tb_service_get_controller_status, "_gpl", "");
KSYMTAB_FUNC(anarchy_ring_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_ring_cleanup, "_gpl", "");
KSYMTAB_FUNC(anarchy_ring_start, "_gpl", "");
KSYMTAB_FUNC(anarchy_ring_stop, "_gpl", "");
KSYMTAB_FUNC(anarchy_ring_transfer, "_gpl", "");
KSYMTAB_FUNC(anarchy_ring_complete, "_gpl", "");
KSYMTAB_FUNC(init_game_compatibility, "_gpl", "");
KSYMTAB_FUNC(cleanup_game_compatibility, "_gpl", "");
KSYMTAB_FUNC(init_game_specific, "_gpl", "");
KSYMTAB_FUNC(setup_game_memory_region, "_gpl", "");
KSYMTAB_FUNC(cleanup_game_memory_region, "_gpl", "");
KSYMTAB_FUNC(init_thermal_monitoring, "_gpl", "");
KSYMTAB_FUNC(cleanup_thermal_monitoring, "_gpl", "");
KSYMTAB_FUNC(anarchy_hotplug_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_hotplug_exit, "_gpl", "");
KSYMTAB_FUNC(anarchy_power_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_power_exit, "_gpl", "");
KSYMTAB_FUNC(anarchy_power_set_profile, "_gpl", "");
KSYMTAB_FUNC(anarchy_power_get_profile, "_gpl", "");
KSYMTAB_FUNC(anarchy_power_set_fan_speed, "_gpl", "");
KSYMTAB_FUNC(anarchy_power_set_power_limit, "_gpl", "");
KSYMTAB_FUNC(anarchy_optimize_for_game, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_set_device_priority, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_transfer, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_transfer_priority, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_cleanup, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_device_start_transfer, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_device_set_burst_size, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_device_set_queue_depth, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_device_enable_prefetch, "_gpl", "");
KSYMTAB_FUNC(anarchy_dma_device_set_write_combining, "_gpl", "");
KSYMTAB_FUNC(init_command_processor, "_gpl", "");
KSYMTAB_FUNC(cleanup_command_processor, "_gpl", "");
KSYMTAB_FUNC(process_game_command, "_gpl", "");
KSYMTAB_FUNC(optimize_command_processing, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_exit, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_disable, "_gpl", "");
KSYMTAB_FUNC(pcie_link_is_up, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_cleanup_state, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_handle_error, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_init_state, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_enable_link, "_gpl", "");
KSYMTAB_FUNC(anarchy_pcie_get_bandwidth_usage, "_gpl", "");
KSYMTAB_FUNC(anarchy_device_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_device_exit, "_gpl", "");
KSYMTAB_FUNC(anarchy_device_suspend, "_gpl", "");
KSYMTAB_FUNC(anarchy_device_resume, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_clock, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_mem_clock, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_power, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_temp, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_fan, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_util, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_mem_util, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_vram_used, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_set_fan_speed, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_set_power_limit, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_set_clocks, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_exit, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_start, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_stop, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_handle_mmio, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_emu_map_memory, "_gpl", "");
KSYMTAB_FUNC(anarchy_perf_exit, "_gpl", "");
KSYMTAB_FUNC(anarchy_perf_init, "_gpl", "");
KSYMTAB_FUNC(anarchy_perf_start, "_gpl", "");
KSYMTAB_FUNC(anarchy_perf_stop, "_gpl", "");
KSYMTAB_FUNC(anarchy_perf_get_state, "_gpl", "");
KSYMTAB_FUNC(init_performance_monitoring, "_gpl", "");
KSYMTAB_FUNC(cleanup_performance_monitoring, "_gpl", "");
KSYMTAB_FUNC(anarchy_service_probe, "_gpl", "");
KSYMTAB_FUNC(anarchy_service_remove, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_set_power_limit, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_get_power_limit, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_power_down, "_gpl", "");
KSYMTAB_FUNC(anarchy_gpu_power_up, "_gpl", "");
KSYMTAB_DATA(anarchy_service_pm, "_gpl", "");

SYMBOL_CRC(anarchy_driver, 0xf0ff9365, "_gpl");
SYMBOL_CRC(anarchy_service_driver, 0x46b4edc4, "_gpl");
SYMBOL_CRC(tb_read32, 0xb232fbd8, "_gpl");
SYMBOL_CRC(tb_write32, 0xbeb2af2b, "_gpl");
SYMBOL_CRC(anarchy_tb_init, 0x27eb1ad0, "_gpl");
SYMBOL_CRC(anarchy_tb_fini, 0xea0af370, "_gpl");
SYMBOL_CRC(anarchy_thunderbolt_init, 0xc0e718db, "_gpl");
SYMBOL_CRC(anarchy_thunderbolt_cleanup, 0x277a79c3, "_gpl");
SYMBOL_CRC(tb_bus_type, 0x7b0b508a, "_gpl");
SYMBOL_CRC(tb_service_driver_register, 0xa459468d, "_gpl");
SYMBOL_CRC(tb_service_driver_unregister, 0x1b2a59aa, "_gpl");
SYMBOL_CRC(tb_service_register_handler, 0x68e26cf8, "_gpl");
SYMBOL_CRC(tb_service_unregister_handler, 0x511ecc09, "_gpl");
SYMBOL_CRC(tb_service_reset_controller, 0xd305c8ef, "_gpl");
SYMBOL_CRC(tb_service_power_cycle_controller, 0xf789d11b, "_gpl");
SYMBOL_CRC(tb_service_get_controller_status, 0xbf9e9cfa, "_gpl");
SYMBOL_CRC(anarchy_ring_init, 0x9e274f0b, "_gpl");
SYMBOL_CRC(anarchy_ring_cleanup, 0x02389edc, "_gpl");
SYMBOL_CRC(anarchy_ring_start, 0xc0d9e7b9, "_gpl");
SYMBOL_CRC(anarchy_ring_stop, 0xeacbb479, "_gpl");
SYMBOL_CRC(anarchy_ring_transfer, 0xccab02fb, "_gpl");
SYMBOL_CRC(anarchy_ring_complete, 0xaa646f6c, "_gpl");
SYMBOL_CRC(init_game_compatibility, 0x3b4614d5, "_gpl");
SYMBOL_CRC(cleanup_game_compatibility, 0x4f1d0a76, "_gpl");
SYMBOL_CRC(init_game_specific, 0xc29244bf, "_gpl");
SYMBOL_CRC(setup_game_memory_region, 0x2e833414, "_gpl");
SYMBOL_CRC(cleanup_game_memory_region, 0x6410b93f, "_gpl");
SYMBOL_CRC(init_thermal_monitoring, 0xc17755dd, "_gpl");
SYMBOL_CRC(cleanup_thermal_monitoring, 0xe3bd4ab2, "_gpl");
SYMBOL_CRC(anarchy_hotplug_init, 0x1fe8a23f, "_gpl");
SYMBOL_CRC(anarchy_hotplug_exit, 0x7b6871e1, "_gpl");
SYMBOL_CRC(anarchy_power_init, 0x4613a863, "_gpl");
SYMBOL_CRC(anarchy_power_exit, 0x2c168146, "_gpl");
SYMBOL_CRC(anarchy_power_set_profile, 0x01b55d9d, "_gpl");
SYMBOL_CRC(anarchy_power_get_profile, 0xccb80afb, "_gpl");
SYMBOL_CRC(anarchy_power_set_fan_speed, 0x9dc08f97, "_gpl");
SYMBOL_CRC(anarchy_power_set_power_limit, 0x9e1dfca5, "_gpl");
SYMBOL_CRC(anarchy_optimize_for_game, 0x0e468638, "_gpl");
SYMBOL_CRC(anarchy_dma_set_device_priority, 0x7f01363a, "_gpl");
SYMBOL_CRC(anarchy_dma_transfer, 0xcee43f94, "_gpl");
SYMBOL_CRC(anarchy_dma_transfer_priority, 0xebc8b455, "_gpl");
SYMBOL_CRC(anarchy_dma_cleanup, 0x912c47bc, "_gpl");
SYMBOL_CRC(anarchy_dma_device_start_transfer, 0xf41d0f44, "_gpl");
SYMBOL_CRC(anarchy_dma_device_set_burst_size, 0x5213eaba, "_gpl");
SYMBOL_CRC(anarchy_dma_device_set_queue_depth, 0x87a64dd0, "_gpl");
SYMBOL_CRC(anarchy_dma_device_enable_prefetch, 0xe061540e, "_gpl");
SYMBOL_CRC(anarchy_dma_device_set_write_combining, 0x37c7a434, "_gpl");
SYMBOL_CRC(init_command_processor, 0xf0870baa, "_gpl");
SYMBOL_CRC(cleanup_command_processor, 0xd815f949, "_gpl");
SYMBOL_CRC(process_game_command, 0x0913ab2e, "_gpl");
SYMBOL_CRC(optimize_command_processing, 0xca009ada, "_gpl");
SYMBOL_CRC(anarchy_pcie_init, 0xc55e5868, "_gpl");
SYMBOL_CRC(anarchy_pcie_exit, 0xdae851f7, "_gpl");
SYMBOL_CRC(anarchy_pcie_disable, 0xb4977395, "_gpl");
SYMBOL_CRC(pcie_link_is_up, 0xa99cb42b, "_gpl");
SYMBOL_CRC(anarchy_pcie_cleanup_state, 0xa5a8b545, "_gpl");
SYMBOL_CRC(anarchy_pcie_handle_error, 0x262bdd30, "_gpl");
SYMBOL_CRC(anarchy_pcie_init_state, 0x55947d0d, "_gpl");
SYMBOL_CRC(anarchy_pcie_enable_link, 0xd0c48460, "_gpl");
SYMBOL_CRC(anarchy_pcie_get_bandwidth_usage, 0x56510833, "_gpl");
SYMBOL_CRC(anarchy_device_init, 0xbb660b9a, "_gpl");
SYMBOL_CRC(anarchy_device_exit, 0xfaa2cb0d, "_gpl");
SYMBOL_CRC(anarchy_device_suspend, 0xaa5a7eb8, "_gpl");
SYMBOL_CRC(anarchy_device_resume, 0x47719fc9, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_clock, 0x39b26f8f, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_mem_clock, 0x90f86290, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_power, 0x19f57f79, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_temp, 0xf1ab24fd, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_fan, 0x8b6a2488, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_util, 0x2df948e5, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_mem_util, 0xdb5a0409, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_vram_used, 0xe0a560e6, "_gpl");
SYMBOL_CRC(anarchy_gpu_set_fan_speed, 0x0d4d4280, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_set_power_limit, 0x740f5d62, "_gpl");
SYMBOL_CRC(anarchy_gpu_set_clocks, 0x8ae3f1b3, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_init, 0x66e33a01, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_exit, 0x3b5c1eb7, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_start, 0xc8622e36, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_stop, 0xee73cd9e, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_handle_mmio, 0x4a683b44, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_map_memory, 0x2de83f5a, "_gpl");
SYMBOL_CRC(anarchy_perf_exit, 0x75641650, "_gpl");
SYMBOL_CRC(anarchy_perf_init, 0x6ad21fcf, "_gpl");
SYMBOL_CRC(anarchy_perf_start, 0x15d8f2d4, "_gpl");
SYMBOL_CRC(anarchy_perf_stop, 0xcca05a79, "_gpl");
SYMBOL_CRC(anarchy_perf_get_state, 0xeb309546, "_gpl");
SYMBOL_CRC(init_performance_monitoring, 0x13bb10df, "_gpl");
SYMBOL_CRC(cleanup_performance_monitoring, 0xee646bce, "_gpl");
SYMBOL_CRC(anarchy_service_probe, 0x591c9287, "_gpl");
SYMBOL_CRC(anarchy_service_remove, 0x8f8d257e, "_gpl");
SYMBOL_CRC(anarchy_gpu_set_power_limit, 0x828f36de, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_power_limit, 0x6887c23f, "_gpl");
SYMBOL_CRC(anarchy_gpu_power_down, 0x00696b24, "_gpl");
SYMBOL_CRC(anarchy_gpu_power_up, 0x989316aa, "_gpl");
SYMBOL_CRC(anarchy_service_pm, 0x3580449e, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\xce\xb0\x1d\xc3"
	"is_vmalloc_addr\0"
	"\x24\x00\x00\x00\xf0\x6f\x22\x5b"
	"pcie_capability_read_word\0\0\0"
	"\x24\x00\x00\x00\xb9\xbf\x28\x2b"
	"pcie_capability_write_word\0\0"
	"\x18\x00\x00\x00\xed\x25\xcd\x49"
	"alloc_workqueue\0"
	"\x1c\x00\x00\x00\x8f\x18\x02\x7f"
	"__msecs_to_jiffies\0\0"
	"\x1c\x00\x00\x00\x91\xc9\xc5\x52"
	"__kmalloc_noprof\0\0\0\0"
	"\x18\x00\x00\x00\xcb\x02\x66\xa4"
	"dev_set_name\0\0\0\0"
	"\x14\x00\x00\x00\xcc\x4f\x26\x8f"
	"pci_iomap\0\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\xb0\x0e\x35\xd5"
	"dma_unmap_page_attrs\0\0\0\0"
	"\x1c\x00\x00\x00\xf3\x76\x04\xd7"
	"device_initialize\0\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x20\x00\x00\x00\x0b\x05\xdb\x34"
	"_raw_spin_lock_irqsave\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x1c\x00\x00\x00\xde\x1e\x40\x39"
	"dev_driver_string\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\x7b\x8b\xee\x5c"
	"dma_map_page_attrs\0\0"
	"\x10\x00\x00\x00\xd8\x7e\x99\x92"
	"_printk\0"
	"\x20\x00\x00\x00\x6d\xb5\xfc\xb2"
	"queue_delayed_work_on\0\0\0"
	"\x20\x00\x00\x00\x68\xd0\xdc\x16"
	"pci_disable_link_state\0\0"
	"\x14\x00\x00\x00\x2c\x4d\x6c\xb0"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x1c\x00\x00\x00\x5e\xd7\xd8\x7c"
	"page_offset_base\0\0\0\0"
	"\x14\x00\x00\x00\x06\x13\xe6\x23"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\xb0\x92\x5c\x85"
	"device_add\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x18\x00\x00\x00\xd6\x82\xea\xfb"
	"tb_service_type\0"
	"\x1c\x00\x00\x00\x0c\xd2\x03\x8c"
	"destroy_workqueue\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x18\x00\x00\x00\xb5\xce\xcb\x44"
	"dma_alloc_attrs\0"
	"\x1c\x00\x00\x00\x48\x51\x27\x07"
	"driver_unregister\0\0\0"
	"\x14\x00\x00\x00\xb0\x28\x9d\x4c"
	"phys_base\0\0\0"
	"\x18\x00\x00\x00\x9f\x0c\xfb\xce"
	"__mutex_init\0\0\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x14\x00\x00\x00\xcc\x25\xd2\xe5"
	"pci_iounmap\0"
	"\x14\x00\x00\x00\x84\x81\x04\xc4"
	"device_del\0\0"
	"\x14\x00\x00\x00\x57\x7b\xb4\x6a"
	"_dev_warn\0\0\0"
	"\x18\x00\x00\x00\x2e\x08\x18\xe1"
	"pcie_get_mps\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\x2b\xe1\xad\xa8"
	"kmemdup_noprof\0\0"
	"\x20\x00\x00\x00\x54\xea\xa5\xd9"
	"__init_waitqueue_head\0\0\0"
	"\x10\x00\x00\x00\x5a\x25\xd5\xe2"
	"strcmp\0\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x18\x00\x00\x00\x6c\x1e\x65\x97"
	"vmemmap_base\0\0\0\0"
	"\x18\x00\x00\x00\x1a\x43\x77\x84"
	"dma_free_attrs\0\0"
	"\x18\x00\x00\x00\x65\x87\xcb\x8a"
	"pcie_get_readrq\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x39\x63\xf4\xc6"
	"init_timer_key\0\0"
	"\x20\x00\x00\x00\xee\xfb\xb4\x10"
	"__kmalloc_cache_noprof\0\0"
	"\x14\x00\x00\x00\x65\x93\x3f\xb4"
	"ktime_get\0\0\0"
	"\x24\x00\x00\x00\x71\x5b\x34\x8c"
	"pcie_capability_read_dword\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x18\x01\x47\x56"
	"__warn_printk\0\0\0"
	"\x20\x00\x00\x00\x6a\xdf\xee\xff"
	"delayed_work_timer_fn\0\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x18\x00\x00\x00\xe5\x70\x24\x39"
	"param_ops_int\0\0\0"
	"\x18\x00\x00\x00\xb5\x12\xda\x61"
	"driver_register\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x18\x00\x00\x00\xaf\xfc\x16\x7b"
	"kmalloc_caches\0\0"
	"\x14\x00\x00\x00\xd3\x85\x33\x2d"
	"system_wq\0\0\0"
	"\x18\x00\x00\x00\xde\x9f\x8a\x25"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "thunderbolt");

MODULE_ALIAS("tbsvc:kBp*v*r*");

MODULE_INFO(srcversion, "4C01F85A3A2B1E7E07BEDF0");
