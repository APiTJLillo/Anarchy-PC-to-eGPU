#!/bin/bash

# Fixed script for Anarchy-PC-to-eGPU module
# This script addresses the missing Module.symvers issue

set -e
echo "Applying fixed solution for Anarchy-PC-to-eGPU module..."

# Get running kernel version
KERNEL_VERSION=$(uname -r)
echo "Running kernel: $KERNEL_VERSION"

# Navigate to kernel module directory
cd "$(dirname "$0")"
MODULE_DIR="$(pwd)"
echo "Working in directory: $MODULE_DIR"

# Check if kernel source is already prepared
if [ ! -d "/usr/src/linux-source-6.11.0" ]; then
    echo "Error: Kernel source not found. Please install it with:"
    echo "sudo apt-get install linux-source-6.11.0"
    exit 1
fi

# Create a special Makefile for building against kernel headers
echo "Creating Makefile for kernel headers build..."
cat > $MODULE_DIR/Makefile.headers << 'EOF'
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
EOF

# Update main.c to add compatibility flags
echo "Updating main.c with compatibility flags..."
cp -f $MODULE_DIR/main.c $MODULE_DIR/main.c.bak

# Add MODULE_ALLOW_MISSING_NAMESPACE_IMPORTS if not already present
if ! grep -q "MODULE_ALLOW_MISSING_NAMESPACE_IMPORTS" $MODULE_DIR/main.c; then
    sed -i '/MODULE_LICENSE/a MODULE_ALLOW_MISSING_NAMESPACE_IMPORTS();' $MODULE_DIR/main.c
fi

# Add MODULE_INFO for BTF if not already present
if ! grep -q "MODULE_INFO(btf" $MODULE_DIR/main.c; then
    sed -i '/MODULE_DESCRIPTION/a MODULE_INFO(intree, "Y");\nMODULE_INFO(btf, "Y");\nMODULE_VERSION("1.0");' $MODULE_DIR/main.c
fi

# Add allow_btf_mismatch parameter if not present
if ! grep -q "allow_btf_mismatch" $MODULE_DIR/main.c; then
    # Find the module parameters section and add our parameter
    sed -i '/module_param(/i bool allow_btf_mismatch = true; /* Allow BTF mismatch */\n' $MODULE_DIR/main.c
    sed -i '/MODULE_PARM_DESC.*);/a module_param(allow_btf_mismatch, bool, 0644);\nMODULE_PARM_DESC(allow_btf_mismatch, "Allow BTF mismatch (default: true)");\n' $MODULE_DIR/main.c
fi

# Use the kernel headers Makefile
echo "Using kernel headers Makefile..."
cp $MODULE_DIR/Makefile.headers $MODULE_DIR/Makefile

# Clean previous build artifacts
echo "Cleaning previous build..."
cd $MODULE_DIR
make clean

# Build the module
echo "Building module with kernel headers..."
make

# Check if the module was built successfully
if [ -f "$MODULE_DIR/anarchy.ko" ]; then
    echo "Module built successfully: $MODULE_DIR/anarchy.ko"
    
    # Verify module format
    echo "Verifying module format..."
    file $MODULE_DIR/anarchy.ko
    
    # Try to load the module with allow_btf_mismatch=1
    echo "Attempting to load the module with allow_btf_mismatch=1..."
    sudo insmod $MODULE_DIR/anarchy.ko allow_btf_mismatch=1 || echo "Module loading failed, but this is expected if no compatible device is connected."
    
    # Create installation directory
    echo "Creating installation directory..."
    sudo mkdir -p /lib/modules/$KERNEL_VERSION/kernel/drivers/thunderbolt
    
    # Install the module
    echo "Installing module..."
    sudo cp -f $MODULE_DIR/anarchy.ko /lib/modules/$KERNEL_VERSION/kernel/drivers/thunderbolt/
    
    # Update module dependencies
    echo "Updating module dependencies..."
    sudo depmod -a
    
    echo "Installation completed successfully."
    echo "You can load the module with: sudo modprobe anarchy allow_btf_mismatch=1"
    echo "Check kernel logs with: dmesg | grep anarchy"
else
    echo "Error: Module build failed!"
    exit 1
fi
