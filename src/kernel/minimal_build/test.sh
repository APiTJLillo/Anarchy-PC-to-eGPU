#!/bin/bash
echo "Testing minimal module..."
sudo insmod anarchy_minimal.ko allow_btf_mismatch=1 || echo "Module loading failed, checking dmesg for details"
sudo dmesg | tail -30
echo "Check if module loaded:"
lsmod | grep anarchy
