# Anarchy PC-to-eGPU Driver

This project provides kernel modules for enabling external GPU support through Thunderbolt connections, allowing a Lenovo P16 to act as an eGPU when connected to a Lenovo Legion Go.

## Project Documentation

- [Integrated Development Roadmap](integrated_roadmap.md) - Comprehensive plan for all project phases
- [Project Status Summary](updated_todos/project_status_summary.md) - Current status, achievements, and next steps
- [Test Mode Documentation](src/kernel/test_mode_solution_documentation.md) - Details about the test mode implementation

## Todo Lists

- [Main Todo Overview](updated_todos/README.md) - Overview of all project phases
- [Phase 1: Foundation and Proof of Concept](updated_todos/phase1/README.md) - 95% Complete
- [Phase 2: Test Mode Enhancement and Core Functionality](updated_todos/phase2/README.md) - Not Started

## Test Mode Solution

A test mode parameter has been implemented to allow the module to load without requiring actual Thunderbolt hardware to be connected. This is essential for development and testing purposes.

### Using Test Mode

```bash
# Build the module
make clean
make

# Load the module with test mode enabled
sudo insmod anarchy.ko test_mode=1

# Verify the module loaded
lsmod | grep anarchy

# Check kernel logs
dmesg | tail -30
```

When the module loads successfully in test mode, you should see these messages in the kernel logs:
```
Anarchy eGPU Driver initializing (test_mode=1)
Anarchy eGPU: Initializing Thunderbolt subsystem
Anarchy eGPU: Running in test mode, skipping Thunderbolt registration
```

### Test Kernel Information

If you encounter module loading issues related to kernel version mismatches, you can try using a newer kernel:

1. Install a newer kernel using the Mainline Kernels app
2. Reboot into the new kernel
3. Verify the kernel version with `uname -r`
4. Install the corresponding headers: `sudo apt-get install linux-headers-$(uname -r)`
5. Rebuild the module against the new kernel headers

The module has been successfully tested with kernel version 6.11.0-24-generic.

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
sudo insmod anarchy.ko
```

### 2. Build with Test Mode Enabled

```bash
make clean
make
sudo insmod anarchy.ko test_mode=1
```

### 3. Build with BTF Mismatch Allowance

If you encounter BTF (BPF Type Format) mismatch errors:

```bash
# Run the script
./build_with_btf_mismatch.sh

# Load the module
sudo insmod anarchy.ko
```

### 4. Direct Build with Exact Configuration

For more precise configuration matching:

```bash
# Run the script
./build_direct_match.sh

# Load the module
sudo insmod anarchy.ko
```

### 5. Full Kernel Source Build

The most thorough approach that builds the module within the kernel source tree:

```bash
# Run the script (may take time to download kernel source)
./build_full_match.sh

# Load the module
sudo insmod anarchy.ko
```

### 6. Configure Kernel to Allow BTF Mismatches

If module loading still fails, you can configure the kernel to allow BTF mismatches:

```bash
# Run as root
sudo ./setup_module_loading.sh

# Try loading the module again
sudo insmod anarchy.ko
```

## Troubleshooting

If you encounter "Invalid module format" errors, it's typically due to one of these issues:

1. **Compiler mismatch**: The kernel was built with a different compiler than what's used for the module
2. **BTF information mismatch**: Different BPF Type Format information
3. **Rust support**: The kernel has Rust support enabled which affects structure layouts
4. **Thunderbolt subsystem not initialized**: Use test_mode=1 to bypass Thunderbolt registration

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

## Current Project Status

The project is currently in Phase 1 (Foundation and Proof of Concept) with approximately 95% completion. The test mode implementation allows development to continue without requiring Thunderbolt hardware to be connected.

See the [Project Status Summary](updated_todos/project_status_summary.md) for more details on current capabilities, limitations, and next steps.
