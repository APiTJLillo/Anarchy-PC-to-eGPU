# Anarchy PC-to-eGPU Driver

This project provides kernel modules for enabling external GPU support through Thunderbolt connections.

## Build Requirements

- Linux kernel headers
- GCC-14 compiler
- Dwarves package (for pahole utility)
- Rust toolchain (to match the kernel build)

## Building & Loading the Module

There are several approaches provided to build and load the module, depending on the compatibility issues you might face:

### 1. Simple Build (try this first)

```bash
make clean
make
sudo insmod src/kernel/anarchy.ko
```

### 2. Build with BTF Mismatch Allowance

If you encounter BTF (BPF Type Format) mismatch errors:

```bash
# Run the script
./build_with_btf_mismatch.sh

# Load the module
sudo insmod src/kernel/anarchy.ko
```

### 3. Direct Build with Exact Configuration

For more precise configuration matching:

```bash
# Run the script
./build_direct_match.sh

# Load the module
sudo insmod src/kernel/anarchy.ko
```

### 4. Full Kernel Source Build

The most thorough approach that builds the module within the kernel source tree:

```bash
# Run the script (may take time to download kernel source)
./build_full_match.sh

# Load the module
sudo insmod src/kernel/anarchy.ko
```

### 5. Configure Kernel to Allow BTF Mismatches

If module loading still fails, you can configure the kernel to allow BTF mismatches:

```bash
# Run as root
sudo ./setup_module_loading.sh

# Try loading the module again
sudo insmod src/kernel/anarchy.ko
```

## Troubleshooting

If you encounter "Invalid module format" errors, it's typically due to one of these issues:

1. **Compiler mismatch**: The kernel was built with a different compiler than what's used for the module
2. **BTF information mismatch**: Different BPF Type Format information
3. **Rust support**: The kernel has Rust support enabled which affects structure layouts

The scripts provided attempt to address all these issues.

## Usage

Once the module is loaded, you can verify with:

```bash
lsmod | grep anarchy
```

For debugging or to view module logs:

```bash
dmesg | grep anarchy
``` 