# Anarchy eGPU Debugging Guide

This guide provides detailed instructions for debugging the Anarchy eGPU system, including common issues, diagnostic procedures, and troubleshooting tools.

## Debug Logging

### Enabling Debug Output

1. **Module Parameters**
```bash
# Load module with debug level 3 (maximum verbosity)
sudo insmod anarchy-egpu.ko debug_level=3

# Or modify at runtime
sudo sh -c 'echo 3 > /sys/module/anarchy_egpu/parameters/debug_level'
```

2. **Kernel Log Levels**
```bash
# Enable all kernel messages
sudo dmesg -n 8

# Clear kernel buffer
sudo dmesg -C

# Watch kernel messages in real-time
sudo dmesg -w
```

## Diagnostic Tools

### 1. Performance Monitor
```bash
# Basic monitoring
sudo ./tools/perf_monitor.py

# High-frequency updates
sudo ./tools/perf_monitor.py --interval 0.1

# Save output to file
sudo ./tools/perf_monitor.py --log perf.log
```

### 2. System Information
```bash
# PCIe information
lspci -vvv

# Thunderbolt controller status
sudo boltctl

# NVIDIA GPU information
nvidia-smi -q
```

### 3. Debug Interface
```bash
# Access debug interface
cat /sys/kernel/debug/anarchy-egpu/status

# List available debug nodes
ls /sys/kernel/debug/anarchy-egpu/
```

## Common Issues and Solutions

### 1. Connection Problems

#### Symptoms
- No device detection
- Intermittent connections
- Connection timeouts

#### Diagnostic Steps
1. Check Thunderbolt status:
```bash
sudo boltctl
dmesg | grep thunderbolt
```

2. Verify PCIe link:
```bash
lspci -vvv | grep -A 10 NVIDIA
```

3. Check connection logs:
```bash
sudo dmesg | grep anarchy
```

#### Solutions
1. Reset Thunderbolt controller:
```bash
sudo ./tools/reset_thunderbolt.sh
```

2. Adjust connection parameters:
```bash
sudo insmod anarchy-egpu.ko tb_retry_count=5 tb_timeout_ms=2000
```

### 2. Performance Issues

#### Symptoms
- Low throughput
- High latency
- Unstable performance

#### Diagnostic Steps
1. Monitor performance metrics:
```bash
sudo ./tools/perf_monitor.py --verbose
```

2. Check DMA status:
```bash
cat /sys/kernel/debug/anarchy-egpu/dma/status
```

3. Verify system resources:
```bash
free -h
vmstat 1
```

#### Solutions
1. Optimize DMA parameters:
```bash
sudo insmod anarchy-egpu.ko ring_buffer_size=512 max_dma_channels=12
```

2. Check for system interference:
```bash
# CPU frequency scaling
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Power management
sudo powertop
```

### 3. Memory Issues

#### Symptoms
- Memory allocation failures
- System instability
- Resource exhaustion

#### Diagnostic Steps
1. Check memory usage:
```bash
cat /sys/kernel/debug/anarchy-egpu/memory/usage
```

2. Monitor system memory:
```bash
watch -n 1 'cat /proc/meminfo'
```

3. Check for memory leaks:
```bash
sudo ./tools/memory_monitor.sh
```

#### Solutions
1. Adjust memory parameters:
```bash
sudo insmod anarchy-egpu.ko max_dma_channels=8 ring_buffer_size=256
```

2. Enable memory debugging:
```bash
sudo insmod anarchy-egpu.ko debug_level=3 debug_memory=1
```

## Advanced Debugging

### 1. Kernel Debugging

#### Using `printk`
```c
// In source code
printk(KERN_DEBUG "anarchy-egpu: DMA transfer status: %d\n", status);
```

#### Using `ftrace`
```bash
# Enable function tracing
echo 1 > /sys/kernel/debug/tracing/events/anarchy-egpu/enable

# View trace
cat /sys/kernel/debug/tracing/trace
```

### 2. Hardware Debugging

#### PCIe Analysis
```bash
# Check link status
sudo lspci -vvv

# Monitor PCIe errors
sudo pcieport-diag -e
```

#### GPU Debugging
```bash
# Enable GPU debug output
export NVIDIA_DEBUG=1

# Check GPU errors
nvidia-smi -q -d ERROR
```

## Debug Symbols and Core Dumps

### Installing Debug Symbols
```bash
# Install debug symbols
sudo apt-get install linux-image-$(uname -r)-dbg

# Load symbols in GDB
(gdb) add-symbol-file anarchy-egpu.ko 0x$(cat /sys/module/anarchy_egpu/sections/.text)
```

### Analyzing Core Dumps
```bash
# Enable core dumps
ulimit -c unlimited

# Analyze with GDB
gdb /usr/src/linux-headers-$(uname -r)/vmlinux /var/crash/core.dump
```

## Performance Profiling

### Using `perf`
```bash
# Record performance data
sudo perf record -a -g ./tools/perf_monitor.py

# Analyze data
sudo perf report
```

### Using `ftrace`
```bash
# Enable function graph tracer
echo function_graph > /sys/kernel/debug/tracing/current_tracer

# Record specific functions
echo 'anarchy_*' > /sys/kernel/debug/tracing/set_ftrace_filter
```

## Best Practices

1. **Systematic Approach**
   - Start with basic diagnostics
   - Isolate the problem area
   - Document findings
   - Test one change at a time

2. **Log Management**
   - Keep detailed logs
   - Use appropriate debug levels
   - Rotate logs regularly
   - Archive important logs

3. **System Health**
   - Monitor system resources
   - Check for interference
   - Verify hardware status
   - Maintain clean environment

4. **Documentation**
   - Record debugging steps
   - Document solutions
   - Update troubleshooting guides
   - Share knowledge 