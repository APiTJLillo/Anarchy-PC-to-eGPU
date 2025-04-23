obj-m := compat_anarchy.o
compat_anarchy-objs := compat_layer.o anarchy_core.o

# Fixed path to anarchy.ko - using the local copy in this directory
anarchy_core.o: $(PWD)/anarchy.ko
	objcopy --redefine-sym init_module=anarchy_init --redefine-sym cleanup_module=anarchy_exit $< $@

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f anarchy_core.o
