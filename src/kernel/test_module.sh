#!/bin/bash
# Script to test the Anarchy eGPU module in test mode

# Load the module with test mode enabled
sudo insmod anarchy.ko test_mode=1

# Check if module loaded successfully
if lsmod | grep -q anarchy; then
    echo "SUCCESS: Module loaded successfully in test mode!"
    echo "Module details:"
    lsmod | grep anarchy
else
    echo "ERROR: Module failed to load. Checking kernel logs..."
    sudo dmesg | tail -30
fi
