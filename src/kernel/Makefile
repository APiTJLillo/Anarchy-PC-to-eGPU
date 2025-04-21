obj-m += anarchy_egpu.o
anarchy_egpu-objs := main.o thunderbolt.o pcie.o anarchy-perf.o

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

# Module dependencies
# KBUILD_EXTRA_SYMBOLS is not needed - kernel symbols are automatically included
KBUILD_MODPOST_WARN := 1

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a

.PHONY: all clean install 