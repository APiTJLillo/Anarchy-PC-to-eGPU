#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "Anarchy eGPU Test Script"

# Parse command line arguments
TEST_TYPE="all"
while [[ $# -gt 0 ]]; do
    case $1 in
        --stress)
            TEST_TYPE="stress"
            shift
            ;;
        --unit)
            TEST_TYPE="unit"
            shift
            ;;
        --integration)
            TEST_TYPE="integration"
            shift
            ;;
        *)
            shift
            ;;
    esac
done

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

# Run the specified tests
case $TEST_TYPE in
    "stress")
        echo -e "\nRunning stress tests..."
        cd "$PROJECT_ROOT/tests/stress"
        if [ -f "connection_interruption_test" ]; then
            echo "Running connection interruption tests..."
            sudo ./connection_interruption_test
        else
            echo "Building connection interruption tests..."
            make
            sudo ./connection_interruption_test
        fi
        ;;
    "unit")
        echo -e "\nRunning unit tests..."
        cd "$PROJECT_ROOT/tests/unit"
        make && ./run_unit_tests
        ;;
    "integration")
        echo -e "\nRunning integration tests..."
        cd "$PROJECT_ROOT/tests/integration"
        make && sudo ./run_integration_tests
        ;;
    "all")
        echo -e "\nRunning all tests..."
        cd "$PROJECT_ROOT/tests"
        make clean && make
        for test_type in unit integration stress; do
            echo "Running $test_type tests..."
            cd "$test_type"
            make && sudo ./run_"$test_type"_tests
            cd ..
        done
        ;;
esac

# Clean up
if [ ! -z "$MONITOR_PID" ]; then
    kill $MONITOR_PID 2>/dev/null || true
fi

echo -e "\nTest complete! Check test results above."
echo "For continuous monitoring, run: $PROJECT_ROOT/tools/perf_monitor.py"
