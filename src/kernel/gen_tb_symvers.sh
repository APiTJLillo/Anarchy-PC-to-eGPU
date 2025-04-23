#!/bin/bash

# Path to the kernel build directory
KBUILD_DIR="/lib/modules/$(uname -r)/build"

# Get the path to the thunderbolt module (handles both .ko and .ko.zst)
TB_MODULE=$(modinfo -n thunderbolt)

if [ -z "$TB_MODULE" ]; then
    echo "Error: Could not find thunderbolt module path"
    exit 1
fi

# Create a temporary directory for working with the module
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT

# If the module is compressed (.ko.zst), decompress it
if [[ "$TB_MODULE" == *.ko.zst ]]; then
    echo "Decompressing thunderbolt module..."
    zstd -d "$TB_MODULE" -o "$TEMP_DIR/thunderbolt.ko"
    TB_MODULE="$TEMP_DIR/thunderbolt.ko"
fi

# Create a clean Module.symvers.thunderbolt file
> Module.symvers.thunderbolt

# Extract symbols from the thunderbolt module using nm
echo "Extracting symbols from thunderbolt module..."
for symbol in tb_bus_type tb_service_driver_register tb_service_driver_unregister; do
    # Use nm to find symbol information
    addr=$(nm "$TB_MODULE" | grep -w "$symbol" | awk '{print $1}')
    if [ -n "$addr" ]; then
        # Format: 0x{addr}\t{symbol}\t{module}\tEXPORT_SYMBOL
        echo "0x$addr\t$symbol\t$TB_MODULE\tEXPORT_SYMBOL" >> Module.symvers.thunderbolt
    else
        echo "Warning: Symbol $symbol not found in thunderbolt module"
    fi
done

# Display the generated file for verification
echo "Generated Module.symvers.thunderbolt:"
cat Module.symvers.thunderbolt

echo "Thunderbolt symbols extraction completed."
