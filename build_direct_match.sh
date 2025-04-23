#!/bin/bash

# Stop on error
set -e

echo "Setting up direct kernel module build with exact matching configuration..."

# Create compiler symbolic link
if [ ! -e /usr/bin/x86_64-linux-gnu-gcc-14 ]; then
    echo "Creating compiler symbolic link..."
    sudo ln -s /usr/bin/gcc-14 /usr/bin/x86_64-linux-gnu-gcc-14
fi

# Clean previous build
echo "Cleaning previous build..."
make clean

# Make sure the system has the right tools
sudo apt-get install -y dwarves

# Check if Rust is installed (since kernel uses it)
if ! command -v rustc &> /dev/null; then
    echo "Rust not found, installing..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source "$HOME/.cargo/env"
fi

# Check if pahole is installed with the right version
PAHOLE_VERSION=$(pahole --version 2>&1 | grep -oP 'v\K[0-9]+' || echo "0")
if [ "$PAHOLE_VERSION" != "127" ]; then
    echo "Warning: Your pahole version ($PAHOLE_VERSION) doesn't match the kernel's version (127)"
    echo "This may cause incompatibility issues"
fi

# Set the correct environment variables for the build
export KBUILD_BUILD_HOST=$(uname -n)
export KBUILD_BUILD_USER=$(whoami)
export KBUILD_BUILD_TIMESTAMP=$(date -R)

# Build the module with matching configuration
echo "Building kernel module with exact matching configuration..."
make -C /lib/modules/$(uname -r)/build \
    M=$(pwd)/src/kernel \
    CONFIG_DEBUG_INFO=y \
    CONFIG_DEBUG_INFO_BTF=y \
    CONFIG_DEBUG_INFO_BTF_MODULES=y \
    CONFIG_PAHOLE_HAS_SPLIT_BTF=y \
    CONFIG_PAHOLE_HAS_LANG_EXCLUDE=y \
    CONFIG_RUST=y \
    CONFIG_PAHOLE_VERSION=127 \
    CONFIG_CC_VERSION_TEXT="x86_64-linux-gnu-gcc-14 (Ubuntu 14.2.0-4ubuntu2) 14.2.0" \
    CONFIG_MODULE_ALLOW_BTF_MISMATCH=y \
    modules

echo "Build completed!"
echo "If the build was successful, you can load the module with:"
echo "sudo insmod src/kernel/anarchy.ko"
echo
echo "If loading still fails, try building with the full kernel match script:"
echo "./build_full_match.sh" 