#!/bin/bash

# Installation script for Anarchy-PC-to-eGPU with BTF and Rust compatibility

set -e
echo "Installing Anarchy eGPU module with BTF and Rust compatibility..."

# Get running kernel version
KERNEL_VERSION=$(uname -r)
echo "Running kernel: $KERNEL_VERSION"

# Navigate to kernel module directory
cd "$(dirname "$0")"
KERNEL_DIR="$(pwd)"
echo "Working in directory: $KERNEL_DIR"

# Check if the module exists
if [ ! -f "anarchy.ko" ]; then
    echo "Error: Module not found. Please run build_with_btf.sh first."
    exit 1
fi

# Create destination directory
INSTALL_DIR="/lib/modules/$KERNEL_VERSION/kernel/drivers/thunderbolt"
echo "Creating installation directory: $INSTALL_DIR"
sudo mkdir -p "$INSTALL_DIR"

# Install the module
echo "Installing module..."
sudo cp -f anarchy.ko "$INSTALL_DIR/"

# Update module dependencies
echo "Updating module dependencies..."
sudo depmod -a

# Create modprobe configuration
echo "Creating modprobe configuration..."
sudo mkdir -p /etc/modprobe.d/
sudo tee /etc/modprobe.d/anarchy.conf > /dev/null << EOF
# Anarchy eGPU Driver configuration
options anarchy power_limit=175 num_dma_channels=8
EOF

# Fix the allow-unsupported.conf file if it exists
if [ -f /etc/modprobe.d/allow-unsupported.conf ]; then
    echo "Fixing allow-unsupported.conf file..."
    sudo cp /etc/modprobe.d/allow-unsupported.conf /etc/modprobe.d/allow-unsupported.conf.backup
    sudo tee /etc/modprobe.d/allow-unsupported.conf > /dev/null << EOF
# Allow loading of unsupported modules
options allow_unsupported_modules=1
EOF
fi

# Create udev rules for automatic loading
echo "Creating udev rules..."
sudo tee /etc/udev/rules.d/99-anarchy-egpu.rules > /dev/null << EOF
# Thunderbolt device rules for Anarchy eGPU
ACTION=="add", SUBSYSTEM=="thunderbolt", ATTR{authorized}=="0", ATTR{authorized}="1"
ACTION=="add", SUBSYSTEM=="thunderbolt", ATTR{device}=="0x0", RUN+="/sbin/modprobe anarchy"
EOF

# Reload udev rules
echo "Reloading udev rules..."
sudo udevadm control --reload-rules

# Load the module
echo "Loading module..."
sudo modprobe thunderbolt
sudo modprobe anarchy || {
    echo "Warning: Failed to load module. This is expected if no compatible device is connected."
    echo "The module will be loaded automatically when a compatible device is connected."
}

echo "Installation completed successfully."
echo "The module will be loaded automatically when a compatible device is connected."
echo "You can also load it manually with: sudo modprobe anarchy"
echo "Check kernel logs with: dmesg | grep anarchy"
