KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: all clean test modules

all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD)/src/kernel modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/src/kernel clean
	rm -f modules.order Module.symvers

test:
	@sudo scripts/run_tests.sh

help:
	@echo "Anarchy eGPU Build System"
	@echo "Available targets:"
	@echo "  all      - Build all modules (default)"
	@echo "  modules  - Build kernel modules"
	@echo "  clean    - Clean build artifacts"
	@echo "  test     - Run test suite"
	@echo "  help     - Show this help message" 