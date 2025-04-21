#!/bin/bash

# Anarchy eGPU End-to-End Test Script

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Test result counters
TESTS_PASSED=0
TESTS_FAILED=0

log_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((TESTS_PASSED++))
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((TESTS_FAILED++))
}

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

# 1. Module Loading Tests
test_module_loading() {
    log_info "Testing module loading..."
    
    # Unload module if already loaded
    if lsmod | grep -q "anarchy_egpu"; then
        modprobe -r anarchy_egpu
    fi
    
    # Test module loading
    if modprobe anarchy_egpu; then
        log_success "Module loaded successfully"
    else
        log_error "Failed to load module"
        return 1
    fi
    
    # Verify module is loaded
    if lsmod | grep -q "anarchy_egpu"; then
        log_success "Module presence verified"
    else
        log_error "Module not present after loading"
        return 1
    fi
    
    return 0
}

# 2. Configuration Tests
test_configuration() {
    log_info "Testing configuration..."
    
    # Check configuration directory
    if [ -d "/etc/anarchy-egpu" ]; then
        log_success "Configuration directory exists"
    else
        log_error "Configuration directory missing"
        return 1
    fi
    
    # Check main config file
    if [ -f "/etc/anarchy-egpu/config.json" ]; then
        log_success "Main configuration file exists"
    else
        log_error "Main configuration file missing"
        return 1
    fi
    
    # Check profiles directory
    if [ -d "/etc/anarchy-egpu/profiles" ]; then
        log_success "Profiles directory exists"
    else
        log_error "Profiles directory missing"
        return 1
    fi
    
    return 0
}

# 3. Thunderbolt Connection Tests
test_thunderbolt_connection() {
    log_info "Testing Thunderbolt connection..."
    
    # Check Thunderbolt controller
    if [ -d "/sys/bus/thunderbolt/devices" ]; then
        log_success "Thunderbolt subsystem available"
    else
        log_error "Thunderbolt subsystem not found"
        return 1
    fi
    
    # Check for connected devices
    if ls /sys/bus/thunderbolt/devices/*/device_name 2>/dev/null; then
        log_success "Thunderbolt devices detected"
    else
        log_error "No Thunderbolt devices found"
        return 1
    fi
    
    # Check XDomain service
    if dmesg | grep -q "Anarchy eGPU: Thunderbolt initialization complete"; then
        log_success "XDomain service initialized"
    else
        log_error "XDomain service initialization failed"
        return 1
    fi
    
    return 0
}

# 4. GPU Detection Tests
test_gpu_detection() {
    log_info "Testing GPU detection..."
    
    # Check NVIDIA driver
    if lsmod | grep -q "nvidia"; then
        log_success "NVIDIA driver loaded"
    else
        log_error "NVIDIA driver not loaded"
        return 1
    fi
    
    # Check for RTX 4090
    if nvidia-smi --query-gpu=gpu_name --format=csv,noheader | grep -q "RTX 4090"; then
        log_success "RTX 4090 detected"
    else
        log_error "RTX 4090 not found"
        return 1
    fi
    
    return 0
}

# 5. DMA Ring Tests
test_dma_rings() {
    log_info "Testing DMA rings..."
    
    # Check ring initialization
    if dmesg | grep -q "anarchy.*ring.*started"; then
        log_success "DMA rings initialized"
    else
        log_error "DMA ring initialization failed"
        return 1
    fi
    
    # Check for ring errors
    if ! dmesg | grep -q "anarchy.*ring.*error"; then
        log_success "No DMA ring errors detected"
    else
        log_error "DMA ring errors found"
        return 1
    fi
    
    return 0
}

# 6. Performance Tests
test_performance() {
    log_info "Testing GPU performance..."
    
    # Run basic CUDA test
    if nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits | grep -q "[1-9]"; then
        log_success "GPU utilization test passed"
    else
        log_error "GPU utilization test failed"
        return 1
    fi
    
    # Check power management
    if nvidia-smi --query-gpu=power.draw --format=csv,noheader,nounits | grep -q "[1-9]"; then
        log_success "Power management test passed"
    else
        log_error "Power management test failed"
        return 1
    fi
    
    return 0
}

# 7. Error Recovery Tests
test_error_recovery() {
    log_info "Testing error recovery..."
    
    # Simulate connection interruption
    modprobe -r anarchy_egpu
    sleep 2
    modprobe anarchy_egpu
    
    # Check recovery
    if dmesg | grep -q "Connection recovery successful"; then
        log_success "Error recovery test passed"
    else
        log_error "Error recovery test failed"
        return 1
    fi
    
    return 0
}

# Main test execution
main() {
    echo "Starting Anarchy eGPU End-to-End Tests"
    echo "======================================="
    
    # Run all tests
    test_module_loading
    test_configuration
    test_thunderbolt_connection
    test_gpu_detection
    test_dma_rings
    test_performance
    test_error_recovery
    
    # Print summary
    echo "======================================="
    echo "Test Summary:"
    echo "Passed: $TESTS_PASSED"
    echo "Failed: $TESTS_FAILED"
    echo "Total: $((TESTS_PASSED + TESTS_FAILED))"
    
    # Return overall status
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        return 0
    else
        echo -e "${RED}Some tests failed!${NC}"
        return 1
    fi
}

# Run main function
main 