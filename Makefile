# Enhanced Makefile with BTF and Rust Support
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

all: prepare_symvers
	@echo "Building with kernel build directory: $(KDIR)"
	@echo "Using compiler: $(CC)"
	$(MAKE) -C $(KDIR) M=$(PWD) modules CONFIG_DEBUG_INFO=y CONFIG_DEBUG_INFO_BTF=y CONFIG_RUST=y V=1

prepare_symvers:
	@echo "Generating Thunderbolt symbols..."
	@./gen_tb_symvers.sh
	@if [ -f "Module.symvers.thunderbolt" ]; then \
		cat Module.symvers.thunderbolt > Module.symvers; \
	fi
	@if [ -f "$(KDIR)/Module.symvers" ]; then \
		cat $(KDIR)/Module.symvers >> Module.symvers; \
	fi

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
