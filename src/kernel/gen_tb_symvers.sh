#!/bin/bash

# Enhanced script to generate Thunderbolt symbols with proper format
# This script ensures proper tab characters are used in the Module.symvers file

# Path to the kernel build directory
KBUILD_DIR="/lib/modules/$(uname -r)/build"

# Get the path to the thunderbolt module
TB_MODULE=$(modinfo -n thunderbolt)

if [ -z "$TB_MODULE" ]; then
    echo "Error: Could not find thunderbolt module path"
    exit 1
fi

# Create a clean Module.symvers.thunderbolt file
> Module.symvers.thunderbolt

# Extract module_layout symbol specifically
echo "Extracting module_layout symbol..."
grep -w "module_layout" "$KBUILD_DIR/Module.symvers" > Module.symvers.module_layout

# List of required thunderbolt symbols
symbols=(
    "tb_bus_type"
    "tb_service_driver_register"
    "tb_service_driver_unregister"
    "tb_service_type"
    "tb_ring_alloc_rx"
    "tb_ring_alloc_tx"
    "tb_ring_start"
    "tb_ring_stop"
    "tb_ring_free"
)

echo "Extracting symbols from kernel Module.symvers..."
for symbol in "${symbols[@]}"; do
    # Get the symbol line from kernel's Module.symvers
    line=$(grep -w "$symbol" "$KBUILD_DIR/Module.symvers" 2>/dev/null)
    
    if [ -n "$line" ]; then
        # Add the symbol to our Module.symvers.thunderbolt using printf to ensure proper tabs
        echo "$line" >> Module.symvers.thunderbolt
    else
        echo "Warning: Symbol $symbol not found in kernel Module.symvers"
        # Try to get it from the running kernel's /proc/kallsyms as fallback
        kallsym=$(grep -w "$symbol" /proc/kallsyms | head -n1)
        if [ -n "$kallsym" ]; then
            addr=$(echo "$kallsym" | awk '{print $1}')
            # Use printf to ensure proper tab characters
            printf "0x%s\t%s\t%s\tEXPORT_SYMBOL_GPL\n" "$addr" "$symbol" "$TB_MODULE" >> Module.symvers.thunderbolt
        fi
    fi
done

echo "Generated Module.symvers.thunderbolt:"
cat Module.symvers.thunderbolt

echo "Generated Module.symvers.module_layout:"
cat Module.symvers.module_layout

echo "Thunderbolt symbols extraction completed."
