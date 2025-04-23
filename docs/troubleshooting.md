# Troubleshooting Guide

## Common Issues and Solutions

### Module Loading Issues

#### Error: "Module verification failed"
**Symptoms:**
- Module fails to load with verification errors
- dmesg shows signature verification failures

**Solutions:**
1. Build the module against your exact kernel version:
   ```bash
   sudo apt-get install linux-headers-$(uname -r)
   make clean
   make
   ```
2. If issue persists, try building with BTF mismatch allowance:
   ```bash
   ./build_with_btf_mismatch.sh
   ```

#### Error: "Unknown symbol in module"
**Symptoms:**
- Module fails to load due to missing symbols
- dmesg shows undefined symbol errors

**Solutions:**
1. Check kernel compatibility:
   ```bash
   uname -r
   ```
2. Ensure you have the correct Thunderbolt support:
   ```bash
   lsmod | grep thunderbolt
   ```
3. Install required dependencies if missing:
   ```bash
   sudo apt-get install linux-modules-extra-$(uname -r)
   ```

### Hardware Detection Issues

#### No Thunderbolt Device Detected
**Symptoms:**
- No devices appear after connection
- dmesg shows no Thunderbolt events

**Solutions:**
1. Check physical connection:
   - Verify cable is properly connected
   - Try reconnecting both ends
   - Test with a different Thunderbolt port
2. Verify Thunderbolt support:
   ```bash
   ls /sys/bus/thunderbolt/devices/
   ```
3. Check BIOS settings:
   - Enable Thunderbolt support
   - Set security level to "No Security" or "User Authorization"
4. Try test mode to verify module functionality:
   ```bash
   sudo insmod anarchy.ko test_mode=1
   ```

#### GPU Not Recognized
**Symptoms:**
- Thunderbolt connection works but GPU isn't detected
- No display output

**Solutions:**
1. Check GPU power status:
   ```bash
   dmesg | grep "GPU power"
   ```
2. Verify NVIDIA driver status:
   ```bash
   nvidia-smi
   ```
3. Check module parameters:
   ```bash
   systool -vm anarchy
   ```

### Performance Issues

#### High Latency
**Symptoms:**
- Delayed response in games
- Poor frame rates

**Solutions:**
1. Check bandwidth usage:
   ```bash
   ./tools/perf_monitor.py --bandwidth
   ```
2. Monitor DMA performance:
   ```bash
   dmesg | grep "DMA"
   ```
3. Optimize settings in configuration:
   ```bash
   sudo systool -m anarchy -A
   ```

#### System Crashes
**Symptoms:**
- System freezes
- Kernel panics

**Solutions:**
1. Check system logs:
   ```bash
   journalctl -b -1 -n 100
   ```
2. Try reducing performance settings:
   - Lower frame buffer size
   - Reduce DMA queue depth
3. Update to latest module version

### Connection Interruption Recovery

#### Handling Unexpected Disconnections
**Symptoms:**
- System freezes after cable disconnect
- GPU doesn't reconnect properly

**Solutions:**
1. Proper disconnection procedure:
   - Stop GPU workload
   - Wait for module to detect disconnect
   - Reconnect cable
2. Recovery commands:
   ```bash
   sudo rmmod anarchy
   sudo modprobe -r thunderbolt
   sudo modprobe thunderbolt
   sudo insmod anarchy.ko
   ```

## Diagnostic Commands

### Module Status
```bash
# Check module loading status
lsmod | grep anarchy

# View module parameters
systool -vm anarchy

# Check kernel logs
dmesg | grep -i "anarchy"
```

### Hardware Status
```bash
# List Thunderbolt devices
ls /sys/bus/thunderbolt/devices/

# Check GPU status
nvidia-smi

# Monitor performance
./tools/perf_monitor.py
```

## Getting Help

If you encounter issues not covered in this guide:

1. Check latest kernel logs:
   ```bash
   dmesg | tail -n 50
   ```

2. Gather system information:
   ```bash
   uname -a
   lspci -v
   lsmod
   ```

3. File an issue with:
   - Complete error messages
   - System information
   - Steps to reproduce
   - Module parameters used
   - Test mode results
