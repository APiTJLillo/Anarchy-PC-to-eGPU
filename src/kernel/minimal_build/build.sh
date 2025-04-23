#!/bin/bash
set -e
echo "Building minimal module..."
make clean
make
echo "Module built successfully."
echo "You can load the module with: sudo insmod anarchy_minimal.ko allow_btf_mismatch=1"
