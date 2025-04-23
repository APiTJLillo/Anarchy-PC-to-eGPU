#!/bin/bash
set -e
echo "Building simplified module..."
make clean
make
echo "Module built successfully."
echo "You can load the module with: sudo insmod anarchy.ko allow_btf_mismatch=1"
