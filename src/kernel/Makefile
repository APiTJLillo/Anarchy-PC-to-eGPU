# Makefile for building against kernel headers with BTF support
KVER := $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build
PWD := $(shell pwd)

# Use the exact same compiler as the kernel
export CC := x86_64-linux-gnu-gcc-14

obj-m := anarchy.o
anarchy-objs := main.o thunderbolt.o thunderbolt_bus.o thunderbolt_driver.o \
                thunderbolt_service.o ring.o game_compat.o thermal.o hotplug.o \
                power_mgmt.o game_opt.o dma.o dma_device.o command_proc.o \
                pcie.o device.o gpu_emu.o perf_monitor.o service_probe.o \
                gpu_power.o service_pm.o

# Enable BTF generation and match Ubuntu kernel configuration
EXTRA_CFLAGS += -g
EXTRA_CFLAGS += -fno-stack-protector -mno-red-zone -mcmodel=kernel
EXTRA_CFLAGS += -DCONFIG_DEBUG_INFO_BTF=1 -DCONFIG_RUST=1

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules CONFIG_DEBUG_INFO=y CONFIG_DEBUG_INFO_BTF=y CONFIG_RUST=y

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
