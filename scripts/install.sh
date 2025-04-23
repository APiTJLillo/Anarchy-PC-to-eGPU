#!/bin/bash
set -e

# Get the absolute path to the project root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "Installing Anarchy eGPU Driver..."
echo "Project root: $PROJECT_ROOT"

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root"
    exit 1
fi

# Get kernel version
KERNEL_VERSION=$(uname -r)
echo "Building for kernel version: $KERNEL_VERSION"

# Build the module
echo "Building kernel module..."
cd "$PROJECT_ROOT/src/kernel"

# Check for required kernel headers
if [ ! -d "/lib/modules/$KERNEL_VERSION/build" ]; then
    echo "Error: Kernel headers not found. Installing linux-headers-$KERNEL_VERSION..."
    apt-get update
    apt-get install -y linux-headers-$KERNEL_VERSION
fi

# Check for required build tools
if ! command -v gcc >/dev/null 2>&1; then
    echo "Installing build tools..."
    apt-get install -y build-essential
fi

# Clean and build
make clean
make -C /lib/modules/$KERNEL_VERSION/build M=$(pwd) modules

# Create thunderbolt directory if it doesn't exist
install -d /lib/modules/$KERNEL_VERSION/kernel/drivers/thunderbolt/

# Install the module
echo "Installing kernel module..."
install -m 644 anarchy.ko /lib/modules/$KERNEL_VERSION/kernel/drivers/thunderbolt/
depmod -a

# Load required modules
echo "Loading required modules..."
modprobe thunderbolt
modprobe nvidia_drm modeset=1  # If using NVIDIA GPU

# Load our module
echo "Loading anarchy module..."
modprobe anarchy

# Setup udev rules for auto-loading
echo 'Installing udev rules...'
cat > /etc/udev/rules.d/99-anarchy-egpu.rules << EOL
# Thunderbolt device rules for Anarchy eGPU
ACTION=="add", SUBSYSTEM=="thunderbolt", ATTR{authorized}=="0", ATTR{authorized}="1"
ACTION=="add", SUBSYSTEM=="thunderbolt", ATTR{device}=="0x0", RUN+="/sbin/modprobe anarchy"
EOL

# Reload udev rules
udevadm control --reload-rules

echo "Installation complete!"
echo "You can now plug in your eGPU."
echo "Monitor the system logs with: dmesg -w"
echo "Monitor performance with: tools/perf_monitor.py"
