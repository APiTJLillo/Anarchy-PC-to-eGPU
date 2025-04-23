#!/bin/bash

# Fixed Compatibility Layer Solution for Anarchy-PC-to-eGPU module
# This script creates a compatibility layer to address the module structure size mismatch
# with a fix for the path issue in the Makefile

set -e
echo "Creating fixed compatibility layer for Anarchy-PC-to-eGPU module..."

# Get running kernel version
KERNEL_VERSION=$(uname -r)
echo "Running kernel: $KERNEL_VERSION"

# Navigate to kernel module directory
cd "$(dirname "$0")"
MODULE_DIR="$(pwd)"
echo "Working in directory: $MODULE_DIR"

# Check if module exists
if [ ! -f "$MODULE_DIR/anarchy.ko" ]; then
    echo "Error: Module anarchy.ko not found. Please build it first with final_fixed_solution.sh"
    exit 1
fi

# Create a directory for the compatibility layer
echo "Creating compatibility layer directory..."
COMPAT_DIR="$MODULE_DIR/compat_layer"
mkdir -p "$COMPAT_DIR"

# Copy the original module to the compatibility layer directory
echo "Copying original module..."
cp "$MODULE_DIR/anarchy.ko" "$COMPAT_DIR/"

# Create compatibility layer source file
echo "Creating compatibility layer source..."
cat > "$COMPAT_DIR/compat_layer.c" << 'EOF'
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anarchy-PC-to-eGPU");
MODULE_DESCRIPTION("Compatibility layer for Anarchy-PC-to-eGPU");
MODULE_VERSION("1.0");
MODULE_INFO(intree, "Y");
MODULE_INFO(btf, "Y");

// Forward declarations for anarchy module functions
extern int anarchy_init(void);
extern void anarchy_exit(void);

static int __init compat_init(void)
{
    printk(KERN_INFO "Anarchy compatibility layer loaded\n");
    return anarchy_init();
}

static void __exit compat_exit(void)
{
    anarchy_exit();
    printk(KERN_INFO "Anarchy compatibility layer unloaded\n");
}

module_init(compat_init);
module_exit(compat_exit);
EOF

# Create Makefile for compatibility layer with FIXED PATH to anarchy.ko
echo "Creating compatibility layer Makefile with fixed path..."
cat > "$COMPAT_DIR/Makefile" << EOF
obj-m := compat_anarchy.o
compat_anarchy-objs := compat_layer.o anarchy_core.o

# Fixed path to anarchy.ko - using the local copy in this directory
anarchy_core.o: \$(PWD)/anarchy.ko
	objcopy --redefine-sym init_module=anarchy_init --redefine-sym cleanup_module=anarchy_exit \$< \$@

all:
	make -C /lib/modules/\$(shell uname -r)/build M=\$(PWD) modules

clean:
	make -C /lib/modules/\$(shell uname -r)/build M=\$(PWD) clean
	rm -f anarchy_core.o
EOF

# Create a build script for the compatibility layer
echo "Creating build script..."
cat > "$COMPAT_DIR/build_compat.sh" << 'EOF'
#!/bin/bash
set -e
echo "Building compatibility layer..."
make clean
make
echo "Compatibility layer built successfully."
echo "You can load the module with: sudo insmod compat_anarchy.ko"
EOF
chmod +x "$COMPAT_DIR/build_compat.sh"

# Create a test script for the compatibility layer
echo "Creating test script..."
cat > "$COMPAT_DIR/test_compat.sh" << 'EOF'
#!/bin/bash
echo "Testing compatibility layer..."
sudo insmod compat_anarchy.ko || echo "Module loading failed, checking dmesg for details"
dmesg | tail -30
echo "Check if module loaded:"
lsmod | grep anarchy
EOF
chmod +x "$COMPAT_DIR/test_compat.sh"

echo "Fixed compatibility layer solution created in $COMPAT_DIR"
echo ""
echo "To build and test the compatibility layer:"
echo "1. cd $COMPAT_DIR"
echo "2. ./build_compat.sh"
echo "3. ./test_compat.sh"
echo ""
echo "This approach creates a thin wrapper module that conforms to your kernel's"
echo "structure expectations while delegating functionality to your original module code."
