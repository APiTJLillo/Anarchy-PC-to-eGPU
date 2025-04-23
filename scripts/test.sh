#!/bin/bash
set -e

echo "Anarchy eGPU Test Script"

# Function to check if module is loaded
check_module() {
    if lsmod | grep -q "^anarchy"; then
        echo "✓ Anarchy module is loaded"
        return 0
    else
        echo "✗ Anarchy module is not loaded"
        return 1
    fi
}

# Function to check Thunderbolt devices
check_thunderbolt() {
    echo "Checking Thunderbolt controller..."
    if [ -d "/sys/bus/thunderbolt/devices" ]; then
        echo "✓ Thunderbolt controller found"
        ls -l /sys/bus/thunderbolt/devices/
    else
        echo "✗ No Thunderbolt controller found"
        return 1
    fi
}

# Function to monitor dmesg output
monitor_dmesg() {
    echo "Monitoring kernel messages (press Ctrl+C to stop)..."
    dmesg -w | grep -i "anarchy\|thunderbolt"
}

# Function to test PCIe link
test_pcie() {
    if [ -e "/sys/bus/pci/devices" ]; then
        echo "Checking PCIe devices..."
        lspci | grep -i "VGA\|3D\|Display"
    fi
}

# Check if module is loaded
if ! lsmod | grep -q "anarchy"; then
    echo "Error: Anarchy module not loaded. Please run install.sh first."
    exit 1
fi

# Check for Thunderbolt devices
echo "Checking Thunderbolt devices..."
tb_devices=$(ls /sys/bus/thunderbolt/devices/)
if [ -z "$tb_devices" ]; then
    echo "No Thunderbolt devices found. Please connect your eGPU."
    exit 1
fi

echo "Found Thunderbolt devices:"
for device in $tb_devices; do
    echo "- $device"
    cat /sys/bus/thunderbolt/devices/$device/device_name 2>/dev/null || echo "  (No device name available)"
    cat /sys/bus/thunderbolt/devices/$device/authorized 2>/dev/null || echo "  (Not authorized)"
done

# Check dmesg for driver messages
echo -e "\nChecking driver messages..."
dmesg | grep -i "anarchy" | tail -n 10

# Launch performance monitor in background
echo -e "\nLaunching performance monitor..."
python3 ../tools/perf_monitor.py &
MONITOR_PID=$!

# Run some basic GPU tests
echo -e "\nRunning basic GPU tests..."
cd ../tests/e2e
make cuda_test
./cuda_test

# Clean up
kill $MONITOR_PID

echo -e "\nTest complete! Check test results above."
echo "For continuous monitoring, run: tools/perf_monitor.py"
