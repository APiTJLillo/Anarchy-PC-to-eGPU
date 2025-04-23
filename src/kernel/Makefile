obj-m := anarchy.o
anarchy-objs := \
	main.o \
	thunderbolt.o \
	thunderbolt_bus.o \
	thunderbolt_driver.o \
	thunderbolt_service.o \
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
	gpu_power.o \
	service_pm.o

KBUILD_EXTRA_SYMBOLS := $(shell find /lib/modules/$(shell uname -r) -name Module.symvers -type f)
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean