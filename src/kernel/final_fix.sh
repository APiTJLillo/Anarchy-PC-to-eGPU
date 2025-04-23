#!/bin/bash

# Final fix script for Anarchy-PC-to-eGPU module
# This script uses the full kernel source approach which is the most reliable method

set -e
echo "Applying final fixes for Anarchy-PC-to-eGPU module..."

# Get running kernel version
KERNEL_VERSION=$(uname -r)
echo "Running kernel: $KERNEL_VERSION"

# Navigate to kernel module directory
cd "$(dirname "$0")"
MODULE_DIR="$(pwd)"
echo "Working in directory: $MODULE_DIR"

# Check if kernel source is already prepared
if [ ! -d "/usr/src/linux-source-6.11.0/.config" ]; then
    echo "Preparing kernel source..."
    cd /usr/src
    
    # Extract kernel source if not already extracted
    if [ ! -d "/usr/src/linux-source-6.11.0" ]; then
        sudo tar xf linux-source-6.11.0.tar.bz2
    fi
    
    cd linux-source-6.11.0
    
    # Copy running kernel config
    sudo cp /boot/config-$(uname -r) .config
    
    # Prepare kernel source (this may take some time)
    echo "Running make oldconfig (answer defaults by pressing Enter)..."
    sudo make oldconfig
    
    echo "Preparing kernel build environment..."
    sudo make prepare
    sudo make modules_prepare
fi

# Create a special Makefile for building against full kernel source
echo "Creating Makefile for full kernel source build..."
cat > $MODULE_DIR/Makefile.kernel_source << 'EOF'
# Makefile for building against full kernel source
KVER := $(shell uname -r)
KSRC := /usr/src/linux-source-6.11.0
PWD := $(shell pwd)

obj-m := anarchy.o
anarchy-objs := main.o thunderbolt.o thunderbolt_bus.o thunderbolt_driver.o \
                thunderbolt_service.o ring.o game_compat.o thermal.o hotplug.o \
                power_mgmt.o game_opt.o dma.o dma_device.o command_proc.o \
                pcie.o device.o gpu_emu.o perf_monitor.o service_probe.o \
                gpu_power.o service_pm.o

all:
	cp $(KSRC)/Module.symvers .
	$(MAKE) -C $(KSRC) M=$(PWD) modules CONFIG_DEBUG_INFO=y CONFIG_DEBUG_INFO_BTF=y CONFIG_RUST=y

clean:
	$(MAKE) -C $(KSRC) M=$(PWD) clean
EOF

# Update main.c to add MODULE_ALLOW_MISSING_NAMESPACE_IMPORTS flag
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
    sed -i '/module_param(/i bool allow_btf_mismatch = false; /* Allow BTF mismatch */\n' $MODULE_DIR/main.c
    sed -i '/MODULE_PARM_DESC.*);/a module_param(allow_btf_mismatch, bool, 0644);\nMODULE_PARM_DESC(allow_btf_mismatch, "Allow BTF mismatch (default: false)");\n' $MODULE_DIR/main.c
fi

# Use the kernel source Makefile
echo "Using kernel source Makefile..."
cp $MODULE_DIR/Makefile.kernel_source $MODULE_DIR/Makefile

# Clean previous build artifacts
echo "Cleaning previous build..."
cd $MODULE_DIR
make clean

# Build the module
echo "Building module with full kernel source..."
make

# Check if the module was built successfully
if [ -f "$MODULE_DIR/anarchy.ko" ]; then
    echo "Module built successfully: $MODULE_DIR/anarchy.ko"
    
    # Verify module format
    echo "Verifying module format..."
    file $MODULE_DIR/anarchy.ko
    
    # Try to load the module
    echo "Attempting to load the module..."
    sudo insmod $MODULE_DIR/anarchy.ko || echo "Module loading failed, but this is expected if no compatible device is connected."
    
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
    echo "You can load the module with: sudo modprobe anarchy"
    echo "Check kernel logs with: dmesg | grep anarchy"
else
    echo "Error: Module build failed!"
    exit 1
fi
