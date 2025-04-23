#!/bin/bash

# Bypass verification solution for Anarchy-PC-to-eGPU module
# This script attempts to bypass module verification to load the module

set -e
echo "Applying bypass verification solution for Anarchy-PC-to-eGPU module..."

# Get running kernel version
KERNEL_VERSION=$(uname -r)
echo "Running kernel: $KERNEL_VERSION"

# Navigate to kernel module directory
cd "$(dirname "$0")"
MODULE_DIR="$(pwd)"
echo "Working in directory: $MODULE_DIR"

# Check if module exists
if [ ! -f "$MODULE_DIR/anarchy.ko" ]; then
    echo "Error: Module anarchy.ko not found. Please build it first with final_fixed_solution.sh"
    exit 1
fi

# Gather diagnostic information
echo "Gathering diagnostic information..."
echo "Module vermagic:"
modinfo $MODULE_DIR/anarchy.ko | grep vermagic
echo "Module signature enforcement:"
cat /proc/sys/kernel/module_sig_enforce
echo "Kernel command line:"
cat /proc/cmdline

# Create a script to temporarily disable module verification
echo "Creating script to temporarily disable module verification..."
cat > $MODULE_DIR/disable_module_verification.sh << 'EOF'
#!/bin/bash
# This script temporarily disables module verification and loads the anarchy module
# WARNING: This is for testing purposes only and reduces system security

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root"
    exit 1
fi

# Backup current settings
echo "Backing up current settings..."
ORIG_SIG_ENFORCE=$(cat /proc/sys/kernel/module_sig_enforce)
ORIG_LOCKDOWN=$(cat /sys/kernel/security/lockdown 2>/dev/null || echo "not_available")

# Temporarily disable module signature enforcement if enabled
if [ -f "/proc/sys/kernel/module_sig_enforce" ] && [ "$(cat /proc/sys/kernel/module_sig_enforce)" -ne 0 ]; then
    echo "Temporarily disabling module signature enforcement..."
    echo 0 > /proc/sys/kernel/module_sig_enforce
fi

# Temporarily disable lockdown if enabled
if [ -f "/sys/kernel/security/lockdown" ]; then
    echo "Temporarily disabling kernel lockdown..."
    echo none > /sys/kernel/security/lockdown
fi

# Try to load the module with force flag
echo "Attempting to load module with force flag..."
insmod $(dirname "$0")/anarchy.ko allow_btf_mismatch=1 || echo "Module loading failed, checking dmesg for details"

# Check if module loaded
if lsmod | grep -q anarchy; then
    echo "Module loaded successfully!"
else
    echo "Module failed to load. Checking kernel logs..."
    dmesg | tail -30
fi

# Restore original settings
echo "Restoring original settings..."
if [ -f "/proc/sys/kernel/module_sig_enforce" ] && [ "$ORIG_SIG_ENFORCE" != "$(cat /proc/sys/kernel/module_sig_enforce)" ]; then
    echo $ORIG_SIG_ENFORCE > /proc/sys/kernel/module_sig_enforce
fi

if [ -f "/sys/kernel/security/lockdown" ] && [ "$ORIG_LOCKDOWN" != "not_available" ]; then
    echo $ORIG_LOCKDOWN > /sys/kernel/security/lockdown
fi

echo "Done. Check dmesg for more information."
EOF

chmod +x $MODULE_DIR/disable_module_verification.sh

# Create a script to modify kernel parameters at boot
echo "Creating script to modify kernel parameters at boot..."
cat > $MODULE_DIR/setup_module_loading.sh << 'EOF'
#!/bin/bash
# This script sets up kernel parameters to allow loading of unsigned modules at boot

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root"
    exit 1
fi

# Create a kernel parameter file
echo "Creating kernel parameter file for module loading..."
cat > /etc/kernel/cmdline.d/99-module-loading.conf << 'EOL'
module.sig_enforce=0 lockdown=none
EOL

# Update GRUB configuration
echo "Updating GRUB configuration..."
if [ -f "/usr/sbin/update-grub" ]; then
    update-grub
elif [ -f "/usr/sbin/grub2-mkconfig" ]; then
    grub2-mkconfig -o /boot/grub2/grub.cfg
else
    echo "Warning: Could not update GRUB configuration automatically"
    echo "Please manually add 'module.sig_enforce=0 lockdown=none' to your kernel parameters"
fi

echo "Kernel parameters have been set up. Please reboot your system for changes to take effect."
echo "After reboot, you can load the module with: sudo modprobe anarchy allow_btf_mismatch=1"
EOF

chmod +x $MODULE_DIR/setup_module_loading.sh

# Create a DKMS configuration
echo "Creating DKMS configuration..."
cat > $MODULE_DIR/dkms.conf << 'EOF'
PACKAGE_NAME="anarchy-egpu"
PACKAGE_VERSION="1.0"
BUILT_MODULE_NAME[0]="anarchy"
DEST_MODULE_LOCATION[0]="/kernel/drivers/thunderbolt"
MAKE[0]="make CONFIG_DEBUG_INFO=y CONFIG_DEBUG_INFO_BTF=y CONFIG_RUST=y"
CLEAN="make clean"
AUTOINSTALL="yes"
EOF

echo "Bypass verification solution completed."
echo ""
echo "You have three options to try loading the module:"
echo ""
echo "1. Temporary solution (for testing):"
echo "   sudo $MODULE_DIR/disable_module_verification.sh"
echo ""
echo "2. Permanent solution (requires reboot):"
echo "   sudo $MODULE_DIR/setup_module_loading.sh"
echo "   # Then reboot and run: sudo modprobe anarchy allow_btf_mismatch=1"
echo ""
echo "3. DKMS solution (for automatic rebuilding with kernel updates):"
echo "   sudo apt-get install dkms"
echo "   sudo mkdir -p /usr/src/anarchy-egpu-1.0"
echo "   sudo cp -r $MODULE_DIR/* /usr/src/anarchy-egpu-1.0/"
echo "   sudo dkms add -m anarchy-egpu -v 1.0"
echo "   sudo dkms build -m anarchy-egpu -v 1.0"
echo "   sudo dkms install -m anarchy-egpu -v 1.0"
echo ""
echo "Check kernel logs with: dmesg | grep anarchy"
