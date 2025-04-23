#!/bin/bash

# Stop on error
set -e

echo "Setting up complete kernel build environment for module..."

# Where to find kernel source - adjust if needed
KERNEL_SOURCE_DIR="/usr/src/linux-source-$(uname -r | cut -d'-' -f1)"
KERNEL_VERSION=$(uname -r)

# Check if kernel source is available
if [ ! -d "$KERNEL_SOURCE_DIR" ]; then
    echo "Kernel source not found at $KERNEL_SOURCE_DIR"
    echo "Installing kernel source package..."
    sudo apt-get update
    sudo apt-get install -y linux-source-$(uname -r | cut -d'-' -f1)
    
    # Extract source if needed
    if [ ! -d "$KERNEL_SOURCE_DIR" ] && [ -f "/usr/src/linux-source-$(uname -r | cut -d'-' -f1).tar.bz2" ]; then
        echo "Extracting kernel source..."
        cd /usr/src
        sudo tar xf linux-source-$(uname -r | cut -d'-' -f1).tar.bz2
    fi
fi

# Create build directory
BUILD_DIR="$PWD/kernel_build"
mkdir -p "$BUILD_DIR"

# Copy kernel config from running kernel
echo "Copying running kernel configuration..."
cp /boot/config-${KERNEL_VERSION} "$BUILD_DIR/.config"

# Create compiler symbolic link
if [ ! -e /usr/bin/x86_64-linux-gnu-gcc-14 ]; then
    echo "Creating compiler symbolic link..."
    sudo ln -s /usr/bin/gcc-14 /usr/bin/x86_64-linux-gnu-gcc-14
fi

# Copy our module to the kernel tree for building
echo "Preparing module source code..."
MODULE_SRC="$PWD/src/kernel"
MODULE_DEST="$BUILD_DIR/drivers/anarchy"

mkdir -p "$MODULE_DEST"
cp -r "$MODULE_SRC"/* "$MODULE_DEST/"

# Create Makefile in the kernel drivers/anarchy directory
cat > "$MODULE_DEST/Makefile" << EOF
obj-m := anarchy.o
anarchy-objs := main.o thunderbolt.o thunderbolt_bus.o thunderbolt_driver.o thunderbolt_service.o ring.o game_compat.o thermal.o hotplug.o power_mgmt.o game_opt.o dma.o dma_device.o command_proc.o pcie.o device.o gpu_emu.o perf_monitor.o service_probe.o gpu_power.o service_pm.o
EOF

# Add our module to the kernel build system
echo "obj-m += anarchy/" >> "$BUILD_DIR/drivers/Makefile"

# Configure and build the module
echo "Building kernel module with exact matching configuration..."
cd "$BUILD_DIR"
yes "" | make oldconfig
make prepare
make scripts
make M=drivers/anarchy modules

# Copy the compiled module back
echo "Copying compiled module..."
cp "$BUILD_DIR/drivers/anarchy/anarchy.ko" "$PWD/src/kernel/"

echo "Build completed!"
echo "If the build was successful, you can load the module with:"
echo "sudo insmod src/kernel/anarchy.ko" 