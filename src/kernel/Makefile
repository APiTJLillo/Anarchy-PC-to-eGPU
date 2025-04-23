# Kernel module name and objects
obj-m := anarchy.o
anarchy-objs := \
	main.o \
	thunderbolt.o \
	thunderbolt_bus.o \
	ring.o \
	game_compat.o \
	thermal.o \
	hotplug.o \
	power_mgmt.o \
	game_opt.o \
	dma.o \
	dma_device.o \
	command_proc.o \
	pcie.o \
	device.o \
	gpu_emu.o \
	perf_monitor.o \
	service_probe.o \
	gpu_power.o

# Include Thunderbolt module symbols
KBUILD_EXTRA_SYMBOLS := $(src)/Module.symvers.thunderbolt

modules:
	cp $(src)/Module.symvers.thunderbolt $(src)/Module.symvers
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules