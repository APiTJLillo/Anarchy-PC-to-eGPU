#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo -e "${YELLOW}Starting Anarchy eGPU Test Suite${NC}"
echo "Project root: $PROJECT_ROOT"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: Please run as root (sudo)${NC}"
    exit 1
fi

# Function to check kernel module status
check_module() {
    local module=$1
    if lsmod | grep -q "^$module"; then
        return 0
    else
        return 1
    fi
}

# Function to clean up modules
cleanup_modules() {
    echo -e "\n${YELLOW}Cleaning up modules...${NC}"
    
    # Remove test module if loaded
    if check_module "pcie_test"; then
        rmmod pcie_test
    fi
    
    # Remove main module if loaded
    if check_module "anarchy_egpu"; then
        rmmod anarchy_egpu
    fi
}

# Cleanup on script exit
trap cleanup_modules EXIT

# Build the modules
echo -e "\n${YELLOW}Building kernel modules...${NC}"
cd "$PROJECT_ROOT/src/kernel"

make clean
if ! make; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Build successful${NC}"

# Remove any existing modules
cleanup_modules

# Run the tests
echo -e "\n${YELLOW}Running PCIe emulation tests...${NC}"

# Load the test module
if ! insmod pcie_test.ko; then
    echo -e "${RED}Failed to load test module!${NC}"
    exit 1
fi

# Wait a moment for tests to complete
sleep 2

# Check test results
echo -e "\n${YELLOW}Test Results:${NC}"
if dmesg | tail -n 50 | grep -q "All PCIe emulation tests passed!"; then
    echo -e "${GREEN}✓ All tests passed${NC}"
else
    echo -e "${RED}✗ Tests failed${NC}"
    echo -e "\nDetailed test output:"
    dmesg | tail -n 50 | grep -E "Testing|Error|Failed|Wrong"
    exit 1
fi

# Print memory mapping info
echo -e "\n${YELLOW}Memory Mapping Information:${NC}"
cat /proc/iomem | grep -A 5 "anarchy"

# Print PCI device info
echo -e "\n${YELLOW}PCI Device Information:${NC}"
lspci -vv | grep -A 10 "NVIDIA"

echo -e "\n${GREEN}Test suite completed successfully${NC}" 