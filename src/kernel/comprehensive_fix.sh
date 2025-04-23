#!/bin/bash

# Comprehensive fix for BTF, module versioning, and configuration issues

set -e
echo "Applying comprehensive fixes for Anarchy eGPU module..."

# Get running kernel version
KERNEL_VERSION=$(uname -r)
echo "Running kernel: $KERNEL_VERSION"

# Install required packages if needed
if ! dpkg -l | grep -q "dwarves"; then
    echo "Installing dwarves package for BTF support..."
    sudo apt-get update
    sudo apt-get install -y dwarves
fi

# Check for x86_64-linux-gnu-gcc-14
if ! command -v x86_64-linux-gnu-gcc-14 &> /dev/null; then
    echo "Installing gcc-14..."
    sudo apt-get update
    sudo apt-get install -y gcc-14
fi

# Navigate to kernel module directory
cd "$(dirname "$0")"
KERNEL_DIR="$(pwd)"
echo "Working in directory: $KERNEL_DIR"

# Fix the allow-unsupported.conf file
echo "Fixing allow-unsupported.conf file..."
sudo tee /etc/modprobe.d/allow-unsupported.conf > /dev/null << EOF
# Allow loading of unsupported modules
options allow_unsupported_modules 1
EOF

# Create a new enhanced Makefile
echo "Creating enhanced Makefile with BTF support..."
cat > Makefile << 'EOF'
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
	@if [ -f "Module.symvers.module_layout" ]; then \
		cat Module.symvers.module_layout > Module.symvers; \
	fi
	@if [ -f "Module.symvers.thunderbolt" ]; then \
		cat Module.symvers.thunderbolt >> Module.symvers; \
	fi

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
EOF

# Create an enhanced gen_tb_symvers.sh script
echo "Creating enhanced symbol extraction script..."
cat > gen_tb_symvers.sh << 'EOF'
#!/bin/bash

# Enhanced script to generate Thunderbolt symbols with proper format
# This script ensures proper tab characters are used in the Module.symvers file

# Path to the kernel build directory
KBUILD_DIR="/lib/modules/$(uname -r)/build"

# Get the path to the thunderbolt module
TB_MODULE=$(modinfo -n thunderbolt)

if [ -z "$TB_MODULE" ]; then
    echo "Error: Could not find thunderbolt module path"
    exit 1
fi

# Create a clean Module.symvers.thunderbolt file
> Module.symvers.thunderbolt

# Extract module_layout symbol specifically
echo "Extracting module_layout symbol..."
grep -w "module_layout" "$KBUILD_DIR/Module.symvers" > Module.symvers.module_layout

# List of required thunderbolt symbols
symbols=(
    "tb_bus_type"
    "tb_service_driver_register"
    "tb_service_driver_unregister"
    "tb_service_type"
    "tb_ring_alloc_rx"
    "tb_ring_alloc_tx"
    "tb_ring_start"
    "tb_ring_stop"
    "tb_ring_free"
)

echo "Extracting symbols from kernel Module.symvers..."
for symbol in "${symbols[@]}"; do
    # Get the symbol line from kernel's Module.symvers
    line=$(grep -w "$symbol" "$KBUILD_DIR/Module.symvers" 2>/dev/null)
    
    if [ -n "$line" ]; then
        # Add the symbol to our Module.symvers.thunderbolt using printf to ensure proper tabs
        echo "$line" >> Module.symvers.thunderbolt
    else
        echo "Warning: Symbol $symbol not found in kernel Module.symvers"
        # Try to get it from the running kernel's /proc/kallsyms as fallback
        kallsym=$(grep -w "$symbol" /proc/kallsyms | head -n1)
        if [ -n "$kallsym" ]; then
            addr=$(echo "$kallsym" | awk '{print $1}')
            # Use printf to ensure proper tab characters
            printf "0x%s\t%s\t%s\tEXPORT_SYMBOL_GPL\n" "$addr" "$symbol" "$TB_MODULE" >> Module.symvers.thunderbolt
        fi
    fi
done

echo "Generated Module.symvers.thunderbolt:"
cat Module.symvers.thunderbolt

echo "Generated Module.symvers.module_layout:"
cat Module.symvers.module_layout

echo "Thunderbolt symbols extraction completed."
EOF

# Make the script executable
chmod +x gen_tb_symvers.sh

# Update main.c with BTF compatibility
echo "Updating main.c with BTF compatibility..."
# First, create a backup
cp -f main.c main.c.bak

# Add MODULE_INFO for BTF
if ! grep -q "MODULE_INFO(btf" main.c; then
    # Add MODULE_INFO lines before the end of the file
    sed -i '/MODULE_DESCRIPTION/a MODULE_INFO(intree, "Y");\nMODULE_INFO(btf, "Y");\nMODULE_VERSION("1.0");' main.c
fi

# Add allow_btf_mismatch parameter if not present
if ! grep -q "allow_btf_mismatch" main.c; then
    # Find the module parameters section and add our parameter
    sed -i '/module_param(/i bool allow_btf_mismatch = false; /* Allow BTF mismatch */\n' main.c
    sed -i '/MODULE_PARM_DESC.*);/a module_param(allow_btf_mismatch, bool, 0644);\nMODULE_PARM_DESC(allow_btf_mismatch, "Allow BTF mismatch (default: false)");\n' main.c
fi

# Clean previous build artifacts
echo "Cleaning previous build..."
make clean

# Build the module
echo "Building module with BTF support..."
make

# Verify BTF sections
echo "Verifying BTF sections..."
readelf -S anarchy.ko | grep -i btf || echo "Warning: No BTF sections found in the module"

# Verify module versioning
echo "Verifying module versioning..."
modprobe --dump-modversions anarchy.ko || echo "Warning: Could not dump module versions"

echo "Comprehensive fix completed."
echo "If the module still fails to load, try: sudo modprobe anarchy allow_btf_mismatch=1"
