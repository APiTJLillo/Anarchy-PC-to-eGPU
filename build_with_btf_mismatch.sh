#!/bin/bash

# Stop on error
set -e

echo "Building kernel module with BTF mismatch allowed..."

# Create compiler symbolic link
if [ ! -e /usr/bin/x86_64-linux-gnu-gcc-14 ]; then
    echo "Creating compiler symbolic link..."
    sudo ln -s /usr/bin/gcc-14 /usr/bin/x86_64-linux-gnu-gcc-14
fi

# Clean previous build
echo "Cleaning previous build..."
make clean

# Install dwarves for pahole
sudo apt-get install -y dwarves

# Build the module with CONFIG_MODULE_ALLOW_BTF_MISMATCH enabled
echo "Building kernel module with BTF mismatch allowed..."
make -C /lib/modules/$(uname -r)/build \
    M=$(pwd)/src/kernel \
    CONFIG_MODULE_ALLOW_BTF_MISMATCH=y \
    CONFIG_DEBUG_INFO=y \
    CONFIG_DEBUG_INFO_BTF=y \
    CONFIG_DEBUG_INFO_BTF_MODULES=y \
    modules

# Reloading the module will require BTF mismatch allowance
echo "Creating modprobe config to allow BTF mismatch..."
echo 'options anarchy allow_btf_mismatch=1' | sudo tee /etc/modprobe.d/anarchy.conf

echo "Build completed!"
echo "If the build was successful, you can load the module with:"
echo "sudo insmod src/kernel/anarchy.ko"
echo
echo "If loading still fails, try running:"
echo "sudo modprobe anarchy allow_btf_mismatch=1" 