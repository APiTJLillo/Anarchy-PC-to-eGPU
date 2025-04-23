#!/bin/bash
set -e
echo "Building compatibility layer..."
make clean
make
echo "Compatibility layer built successfully."
echo "You can load the module with: sudo insmod compat_anarchy.ko"
