#!/bin/bash

# Stop on error
set -e

echo "Setting up build environment for kernel module..."

# Create compiler symbolic link if needed
if [ ! -e /usr/bin/x86_64-linux-gnu-gcc-14 ]; then
    echo "Creating compiler symbolic link..."
    sudo ln -s /usr/bin/gcc-14 /usr/bin/x86_64-linux-gnu-gcc-14
fi

# Clean previous build
echo "Cleaning previous build..."
make clean

# Get kernel config from running kernel
echo "Getting kernel configuration from running kernel..."
KERNEL_VERSION=$(uname -r)
cp /boot/config-${KERNEL_VERSION} .config

# Build the module with proper configuration
echo "Building kernel module with matching configuration..."
make modules CONFIG_DEBUG_INFO=y CONFIG_DEBUG_INFO_BTF=y CONFIG_DEBUG_INFO_BTF_MODULES=y CONFIG_PAHOLE_HAS_SPLIT_BTF=y CONFIG_PAHOLE_HAS_LANG_EXCLUDE=y CONFIG_RUST=y V=1

echo "Build completed!"
echo "If the build was successful, you can load the module with:"
echo "sudo insmod src/kernel/anarchy.ko" 