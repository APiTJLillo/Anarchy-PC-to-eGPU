#!/bin/bash

# Enhanced build script for Anarchy-PC-to-eGPU with BTF and Rust compatibility

set -e
echo "Building Anarchy eGPU module with BTF and Rust compatibility..."

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

# Make scripts executable
chmod +x gen_tb_symvers.sh

# Copy kernel's Module.symvers
echo "Copying kernel Module.symvers..."
cp -f /lib/modules/$KERNEL_VERSION/build/Module.symvers Module.symvers.kernel
cat Module.symvers.kernel > Module.symvers

# Clean previous build artifacts
echo "Cleaning previous build..."
make clean

# Build the module
echo "Building module..."
make

echo "Build completed. Check for any errors above."

# Check if the module was built successfully
if [ -f "anarchy.ko" ]; then
    echo "Module built successfully: $(pwd)/anarchy.ko"
else
    echo "Error: Module build failed!"
    exit 1
fi
