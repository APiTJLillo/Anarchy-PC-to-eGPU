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
	gpu_power.o \
	bandwidth.o \
	pcie_link.o \
	pcie_mon.o

# Kernel build directory and current directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Include paths
ccflags-y := -I$(src) -DDEBUG

# Debug flags
ccflags-$(CONFIG_ANARCHY_EGPU_DEBUG) += -DDEBUG

# Configuration
CONFIG_ANARCHY_EGPU=m
CONFIG_ANARCHY_EGPU_DEBUG=y

# Architecture detection
ARCH := $(shell uname -m | sed -e s/i.86/x86/ -e s/x86_64/x86/)

# Kernel build setup
EXTRA_CFLAGS += -I$(src)/include
EXTRA_CFLAGS += -I$(KDIR)/arch/$(ARCH)/include
EXTRA_CFLAGS += -I$(KDIR)/arch/$(ARCH)/include/generated
EXTRA_CFLAGS += -I$(KDIR)/include/uapi
EXTRA_CFLAGS += -I$(KDIR)/include/generated/uapi

# Module versioning
EXTRA_CFLAGS += -DDRIVER_VERSION=\"0.1.0\"

# Enable kernel module hardening
EXTRA_CFLAGS += -fno-pic
EXTRA_LDFLAGS += -z noexecstack

# Suppress all warnings
EXTRA_CFLAGS += -w

# Add Thunderbolt module dependency
KBUILD_EXTRA_SYMBOLS := $(KDIR)/Module.symvers

all: gen_symvers
	@$(MAKE) -s -C $(KDIR) M=$(PWD) modules 2>&1 | grep -v "warning:" | grep -v "note:" | grep -v "the compiler differs"

gen_symvers:
	@./gen_tb_symvers.sh

clean:
	@$(MAKE) -s -C $(KDIR) M=$(PWD) clean
	@rm -f *.o *.ko *.mod.c *.mod.o *.symvers *.order

install: all
	@$(MAKE) -s -C $(KDIR) M=$(PWD) modules_install
	@depmod -a

.PHONY: all clean install gen_symvers