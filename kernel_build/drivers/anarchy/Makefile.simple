# Simple kernel module Makefile
KVER := $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build

obj-m := anarchy.o
anarchy-objs := main.o thunderbolt.o thunderbolt_bus.o thunderbolt_driver.o \
                thunderbolt_service.o ring.o game_compat.o thermal.o hotplug.o \
                power_mgmt.o game_opt.o dma.o dma_device.o command_proc.o \
                pcie.o device.o gpu_emu.o perf_monitor.o service_probe.o \
                gpu_power.o service_pm.o

EXTRA_CFLAGS += -DDEBUG -g -O1

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules CONFIG_DEBUG_INFO=y V=1

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
