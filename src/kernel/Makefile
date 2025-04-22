obj-m += anarchy_egpu.o
anarchy_egpu-objs := main.o thunderbolt.o pcie.o \
                    command_proc.o dma.o game_compat.o \
                    game_opt.o power_mgmt.o perf_monitor.o \
                    usb4_device.o hotplug.o thermal.o bandwidth.o \
                    device.o device_remove.o anarchy-perf.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Include paths
ccflags-y := -I$(src)/include -I$(src)/../include

# Debug flags
ccflags-$(CONFIG_ANARCHY_EGPU_DEBUG) += -DDEBUG

# Warning flags (excluding ones that cause issues with kernel code)
ccflags-y += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

ccflags-y := -I$(src)/../../include

# Debug flags
ccflags-$(CONFIG_ANARCHY_EGPU_DEBUG) += -DDEBUG

# Version
ccflags-y += -DDRIVER_VERSION=\"0.1.0\"

# Thunderbolt dependency
CFLAGS_thunderbolt.o := -I$(src)/../../include -Wno-missing-braces
ccflags-y += -I$(KDIR)/drivers/thunderbolt

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

# Debug configuration
ccflags-$(CONFIG_ANARCHY_EGPU_DEBUG) += -DDEBUG

# Module versioning
EXTRA_CFLAGS += -DDRIVER_VERSION=\"0.1.0\"

# Enable kernel module hardening
EXTRA_CFLAGS += -fno-pic
EXTRA_LDFLAGS += -z noexecstack

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a

.PHONY: all clean install 