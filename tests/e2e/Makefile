ccflags-y := -I$(src)/../../include

obj-m := test_thunderbolt.o
test_thunderbolt-y := test_thunderbolt_main.o ../../src/kernel/thunderbolt.o ../../src/kernel/pcie.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean 