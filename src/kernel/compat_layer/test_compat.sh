#!/bin/bash
echo "Testing compatibility layer..."
sudo insmod compat_anarchy.ko || echo "Module loading failed, checking dmesg for details"
dmesg | tail -30
echo "Check if module loaded:"
lsmod | grep anarchy
