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

SYMBOL_CRC(anarchy_driver, 0xa5e5474b, "_gpl");
SYMBOL_CRC(anarchy_service_driver, 0x124d7894, "_gpl");
SYMBOL_CRC(tb_read32, 0x2138b2b0, "_gpl");
SYMBOL_CRC(tb_write32, 0x9b8963b5, "_gpl");
SYMBOL_CRC(anarchy_tb_init, 0x67a5f3a4, "_gpl");
SYMBOL_CRC(anarchy_tb_fini, 0xca1fa683, "_gpl");
SYMBOL_CRC(anarchy_thunderbolt_init, 0xc0e718db, "_gpl");
SYMBOL_CRC(anarchy_thunderbolt_cleanup, 0x277a79c3, "_gpl");
SYMBOL_CRC(tb_bus_type, 0x89feb8ad, "_gpl");
SYMBOL_CRC(tb_service_driver_register, 0x81214f9a, "_gpl");
SYMBOL_CRC(tb_service_driver_unregister, 0xb706472d, "_gpl");
SYMBOL_CRC(tb_service_register_handler, 0x1bd1f205, "_gpl");
SYMBOL_CRC(tb_service_unregister_handler, 0xbacda98a, "_gpl");
SYMBOL_CRC(tb_service_reset_controller, 0x64ec6e2b, "_gpl");
SYMBOL_CRC(tb_service_power_cycle_controller, 0x3e2c445d, "_gpl");
SYMBOL_CRC(tb_service_get_controller_status, 0x9c3b1eca, "_gpl");
SYMBOL_CRC(anarchy_ring_init, 0x5124ac5a, "_gpl");
SYMBOL_CRC(anarchy_ring_cleanup, 0x7f4becae, "_gpl");
SYMBOL_CRC(anarchy_ring_start, 0x0f131404, "_gpl");
SYMBOL_CRC(anarchy_ring_stop, 0x7cdfb60c, "_gpl");
SYMBOL_CRC(anarchy_ring_transfer, 0xe6b39361, "_gpl");
SYMBOL_CRC(anarchy_ring_complete, 0xddf9bb13, "_gpl");
SYMBOL_CRC(init_game_compatibility, 0x7311d0b0, "_gpl");
SYMBOL_CRC(cleanup_game_compatibility, 0xcc20ec1d, "_gpl");
SYMBOL_CRC(init_game_specific, 0x4f9d68f8, "_gpl");
SYMBOL_CRC(setup_game_memory_region, 0xf5ddeb98, "_gpl");
SYMBOL_CRC(cleanup_game_memory_region, 0xf82972f3, "_gpl");
SYMBOL_CRC(init_thermal_monitoring, 0x4865efcf, "_gpl");
SYMBOL_CRC(cleanup_thermal_monitoring, 0x829a7104, "_gpl");
SYMBOL_CRC(anarchy_hotplug_init, 0xe0d18a92, "_gpl");
SYMBOL_CRC(anarchy_hotplug_exit, 0x935a5fab, "_gpl");
SYMBOL_CRC(anarchy_power_init, 0x9ded03cc, "_gpl");
SYMBOL_CRC(anarchy_power_exit, 0x9ab5f9b7, "_gpl");
SYMBOL_CRC(anarchy_power_set_profile, 0x94c39a2f, "_gpl");
SYMBOL_CRC(anarchy_power_get_profile, 0x6964d51f, "_gpl");
SYMBOL_CRC(anarchy_power_set_fan_speed, 0xe8d4b4e6, "_gpl");
SYMBOL_CRC(anarchy_power_set_power_limit, 0x5b681f6e, "_gpl");
SYMBOL_CRC(anarchy_optimize_for_game, 0x90221039, "_gpl");
SYMBOL_CRC(anarchy_dma_set_device_priority, 0xbad8de08, "_gpl");
SYMBOL_CRC(anarchy_dma_transfer, 0xd06f89a8, "_gpl");
SYMBOL_CRC(anarchy_dma_transfer_priority, 0xcbb9067d, "_gpl");
SYMBOL_CRC(anarchy_dma_cleanup, 0x2d983997, "_gpl");
SYMBOL_CRC(anarchy_dma_device_start_transfer, 0x1c46db85, "_gpl");
SYMBOL_CRC(anarchy_dma_device_set_burst_size, 0x10fc7815, "_gpl");
SYMBOL_CRC(anarchy_dma_device_set_queue_depth, 0xdf7767f9, "_gpl");
SYMBOL_CRC(anarchy_dma_device_enable_prefetch, 0xa58ebb80, "_gpl");
SYMBOL_CRC(anarchy_dma_device_set_write_combining, 0xb6cf49d3, "_gpl");
SYMBOL_CRC(init_command_processor, 0xd04dec8f, "_gpl");
SYMBOL_CRC(cleanup_command_processor, 0x9c9aa8fb, "_gpl");
SYMBOL_CRC(process_game_command, 0xc5307c97, "_gpl");
SYMBOL_CRC(optimize_command_processing, 0xa5be584d, "_gpl");
SYMBOL_CRC(anarchy_pcie_init, 0xe63dd852, "_gpl");
SYMBOL_CRC(anarchy_pcie_exit, 0x43807356, "_gpl");
SYMBOL_CRC(anarchy_pcie_disable, 0x064dbfe2, "_gpl");
SYMBOL_CRC(pcie_link_is_up, 0x92040dd1, "_gpl");
SYMBOL_CRC(anarchy_pcie_cleanup_state, 0x2bc23f56, "_gpl");
SYMBOL_CRC(anarchy_pcie_handle_error, 0xb5a3d8ce, "_gpl");
SYMBOL_CRC(anarchy_pcie_init_state, 0x729686a0, "_gpl");
SYMBOL_CRC(anarchy_pcie_enable_link, 0xd0592282, "_gpl");
SYMBOL_CRC(anarchy_pcie_get_bandwidth_usage, 0x8dd51054, "_gpl");
SYMBOL_CRC(anarchy_device_init, 0x6273b4ea, "_gpl");
SYMBOL_CRC(anarchy_device_exit, 0xa533c76d, "_gpl");
SYMBOL_CRC(anarchy_device_suspend, 0x123c875f, "_gpl");
SYMBOL_CRC(anarchy_device_resume, 0xf9787541, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_clock, 0x8db12098, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_mem_clock, 0xc562d6cf, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_power, 0x694b351b, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_temp, 0x549550b4, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_fan, 0x0403c53f, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_util, 0xf3ea6521, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_mem_util, 0x922a7986, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_vram_used, 0x868d3e38, "_gpl");
SYMBOL_CRC(anarchy_gpu_set_fan_speed, 0x07ec371c, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_set_power_limit, 0x111d0769, "_gpl");
SYMBOL_CRC(anarchy_gpu_set_clocks, 0x73471a0e, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_init, 0x64204ccd, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_exit, 0xa9bc2cbc, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_start, 0x698b2d18, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_stop, 0xab4eb161, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_handle_mmio, 0x9ef5cf03, "_gpl");
SYMBOL_CRC(anarchy_gpu_emu_map_memory, 0x46d1d07a, "_gpl");
SYMBOL_CRC(anarchy_perf_exit, 0xbee3d1a6, "_gpl");
SYMBOL_CRC(anarchy_perf_init, 0x1b5e7aa2, "_gpl");
SYMBOL_CRC(anarchy_perf_start, 0xafc477db, "_gpl");
SYMBOL_CRC(anarchy_perf_stop, 0x8cd05141, "_gpl");
SYMBOL_CRC(anarchy_perf_get_state, 0xab034712, "_gpl");
SYMBOL_CRC(init_performance_monitoring, 0x9b5126ca, "_gpl");
SYMBOL_CRC(cleanup_performance_monitoring, 0x45e7e332, "_gpl");
SYMBOL_CRC(anarchy_service_probe, 0x514d04e8, "_gpl");
SYMBOL_CRC(anarchy_service_remove, 0x00a49eae, "_gpl");
SYMBOL_CRC(anarchy_gpu_set_power_limit, 0xee19d6fa, "_gpl");
SYMBOL_CRC(anarchy_gpu_get_power_limit, 0x9ae4c1d9, "_gpl");
SYMBOL_CRC(anarchy_gpu_power_down, 0x7e72c91d, "_gpl");
SYMBOL_CRC(anarchy_gpu_power_up, 0xc4fdb703, "_gpl");
SYMBOL_CRC(anarchy_service_pm, 0x0c0228d9, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\xce\xb0\x1d\xc3"
	"is_vmalloc_addr\0"
	"\x24\x00\x00\x00\xfa\x78\x95\xaa"
	"pcie_capability_read_word\0\0\0"
	"\x24\x00\x00\x00\xa0\xa1\x0b\xf0"
	"pcie_capability_write_word\0\0"
	"\x18\x00\x00\x00\xed\x25\xcd\x49"
	"alloc_workqueue\0"
	"\x1c\x00\x00\x00\x8f\x18\x02\x7f"
	"__msecs_to_jiffies\0\0"
	"\x1c\x00\x00\x00\x91\xc9\xc5\x52"
	"__kmalloc_noprof\0\0\0\0"
	"\x18\x00\x00\x00\x0b\x37\x55\x1e"
	"dev_set_name\0\0\0\0"
	"\x14\x00\x00\x00\xf4\x98\x44\xe4"
	"pci_iomap\0\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\x36\xea\xd2\x02"
	"dma_unmap_page_attrs\0\0\0\0"
	"\x1c\x00\x00\x00\x19\x56\xed\xe3"
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
	"\x1c\x00\x00\x00\x7e\xf1\x2b\xaa"
	"dev_driver_string\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\x69\x45\x94\xe4"
	"dma_map_page_attrs\0\0"
	"\x10\x00\x00\x00\xd8\x7e\x99\x92"
	"_printk\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x20\x00\x00\x00\x6d\xb5\xfc\xb2"
	"queue_delayed_work_on\0\0\0"
	"\x20\x00\x00\x00\xb8\xf6\x41\x73"
	"pci_disable_link_state\0\0"
	"\x14\x00\x00\x00\xed\x5e\x1f\xcc"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x1c\x00\x00\x00\x5e\xd7\xd8\x7c"
	"page_offset_base\0\0\0\0"
	"\x14\x00\x00\x00\xd6\xe0\x4e\x41"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\xb8\xf6\xec\xc4"
	"device_add\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x18\x00\x00\x00\xf8\x61\x33\xc4"
	"tb_service_type\0"
	"\x1c\x00\x00\x00\x0c\xd2\x03\x8c"
	"destroy_workqueue\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x18\x00\x00\x00\x2e\xcd\xf9\x2a"
	"dma_alloc_attrs\0"
	"\x1c\x00\x00\x00\x9c\xec\x22\xe2"
	"driver_unregister\0\0\0"
	"\x14\x00\x00\x00\xb0\x28\x9d\x4c"
	"phys_base\0\0\0"
	"\x18\x00\x00\x00\x9f\x0c\xfb\xce"
	"__mutex_init\0\0\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x14\x00\x00\x00\x3c\x6e\x3f\x0d"
	"pci_iounmap\0"
	"\x14\x00\x00\x00\xe5\x8f\x5f\xb8"
	"device_del\0\0"
	"\x14\x00\x00\x00\x4b\x94\x99\xed"
	"_dev_warn\0\0\0"
	"\x18\x00\x00\x00\xd8\x0d\x6b\xeb"
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
	"\x18\x00\x00\x00\xd1\xed\x36\x0a"
	"dma_free_attrs\0\0"
	"\x18\x00\x00\x00\x5c\x88\x4d\x25"
	"pcie_get_readrq\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x39\x63\xf4\xc6"
	"init_timer_key\0\0"
	"\x20\x00\x00\x00\xee\xc4\x9e\x76"
	"__kmalloc_cache_noprof\0\0"
	"\x14\x00\x00\x00\x65\x93\x3f\xb4"
	"ktime_get\0\0\0"
	"\x24\x00\x00\x00\x65\x72\xdf\xea"
	"pcie_capability_read_dword\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x18\x01\x47\x56"
	"__warn_printk\0\0\0"
	"\x20\x00\x00\x00\x6a\xdf\xee\xff"
	"delayed_work_timer_fn\0\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x18\x00\x00\x00\x74\xdc\x23\xae"
	"param_ops_int\0\0\0"
	"\x18\x00\x00\x00\x86\x7c\x5a\x6d"
	"driver_register\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x18\x00\x00\x00\x32\xa5\x35\xfe"
	"kmalloc_caches\0\0"
	"\x14\x00\x00\x00\xd3\x85\x33\x2d"
	"system_wq\0\0\0"
	"\x18\x00\x00\x00\xfd\x1a\x4a\xa7"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "thunderbolt");

MODULE_ALIAS("tbsvc:kBp*v*r*");

MODULE_INFO(srcversion, "DD98A0A727BCCD75E58F0EC");
