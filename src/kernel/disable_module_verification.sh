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
ORIG_LOCKDOWN=$(cat /sys/kernel/security/lockdown 2>/dev/null || echo "not_available")

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
if [ -f "/sys/kernel/security/lockdown" ] && [ "$ORIG_LOCKDOWN" != "not_available" ]; then
    echo $ORIG_LOCKDOWN > /sys/kernel/security/lockdown
fi

echo "Done. Check dmesg for more information."
