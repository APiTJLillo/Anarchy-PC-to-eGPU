#!/bin/bash
# Script to build the Anarchy eGPU module with test mode support

# Copy the test mode version of main.c
cp main.c.test_mode main.c

# Clean previous build artifacts
make clean

# Build the module
make

echo "Build complete. To load the module with test mode enabled, run:"
echo "sudo insmod anarchy.ko test_mode=1"
