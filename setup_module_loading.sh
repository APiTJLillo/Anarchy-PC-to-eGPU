#!/bin/bash

# Stop on error
set -e

echo "Setting up kernel for loading modules with BTF mismatches..."

# Check if running as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit 1
fi

# Create a kernel parameter file to allow BTF mismatches
echo "Creating kernel parameter file for module loading with BTF mismatches..."
cat > /etc/sysctl.d/99-module-allow-btf-mismatch.conf << EOF
# Allow loading of modules with BTF mismatches
kernel.module_allow_btf_mismatch = 1
EOF

# Load the parameter immediately
echo "Applying kernel parameter..."
sysctl -p /etc/sysctl.d/99-module-allow-btf-mismatch.conf

echo "Setup completed!"
echo "You should now be able to load kernel modules with BTF mismatches."
echo "Try loading your module with:"
echo "sudo insmod src/kernel/anarchy.ko" 