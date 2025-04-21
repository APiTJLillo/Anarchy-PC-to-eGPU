# Anarchy eGPU Installation Guide

## Prerequisites

### Hardware Requirements
- Lenovo P16 laptop with Thunderbolt 4 port
- Legion Go with RTX 4090
- Thunderbolt 4/USB4 cable (40Gbps certified)
- Adequate power supply for RTX 4090 (minimum 850W recommended)

### Software Requirements
- Linux kernel 5.10 or newer
- NVIDIA drivers 535.x or newer
- GCC/Clang compiler
- Linux kernel headers
- Build tools (make, etc.)

## Installation Steps

### 1. Prepare the System

```bash
# Install required packages
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r) dkms nvidia-driver-535

# Create installation directory
sudo mkdir -p /usr/local/src/anarchy-egpu
```

### 2. Build and Install the Kernel Module

```bash
# Clone the repository
git clone https://github.com/your-username/anarchy-egpu.git
cd anarchy-egpu

# Build the module
make

# Install the module
sudo make install

# Load the module
sudo modprobe anarchy-egpu
```

### 3. Configure the System

```bash
# Create configuration directory
sudo mkdir -p /etc/anarchy-egpu

# Copy default configuration
sudo cp config/default.json /etc/anarchy-egpu/config.json
sudo cp config/profiles/* /etc/anarchy-egpu/profiles/
```

### 4. Set Up Automatic Loading

```bash
# Add module to load at boot
echo "anarchy-egpu" | sudo tee /etc/modules-load.d/anarchy-egpu.conf

# Configure module parameters (optional)
echo "options anarchy-egpu auto_connect=1" | sudo tee /etc/modprobe.d/anarchy-egpu.conf
```

## Configuration

### Basic Configuration
The default configuration file is located at `/etc/anarchy-egpu/config.json`. You can modify this file to change:
- Auto-connection settings
- Power management preferences
- Monitoring thresholds
- Default performance profile

### Performance Profiles
Three default profiles are provided:
1. **power_saver**: Optimized for battery life
   - Reduced power limit (250W)
   - Conservative fan curve
   - Underclocked GPU

2. **balanced**: Default profile
   - Moderate power limit (300W)
   - Balanced fan curve
   - Stock GPU clocks

3. **performance**: Maximum performance
   - High power limit (350W)
   - Aggressive fan curve
   - Overclocked GPU

Custom profiles can be added to `/etc/anarchy-egpu/profiles/`.

## Verification

### Check Module Status
```bash
# Verify module is loaded
lsmod | grep anarchy

# Check kernel log for module messages
dmesg | grep anarchy

# View module information
modinfo anarchy-egpu
```

### Test Connection
1. Connect the Thunderbolt cable between P16 and Legion Go
2. Check system log for connection status
3. Verify GPU detection in NVIDIA settings
4. Run basic GPU test (e.g., nvidia-smi)

## Troubleshooting

### Common Issues

1. **Module fails to load**
   - Check kernel version compatibility
   - Verify all dependencies are installed
   - Check kernel log for specific errors

2. **Connection issues**
   - Verify Thunderbolt cable is properly connected
   - Check Thunderbolt port authorization
   - Ensure both devices are powered on
   - Check system log for connection errors

3. **Performance problems**
   - Verify power supply is adequate
   - Check thermal conditions
   - Monitor GPU utilization and power draw
   - Review current performance profile settings

### Debug Logging

To enable debug logging:
```bash
# Enable debug output
echo "module anarchy-egpu +p" | sudo tee /sys/kernel/debug/dynamic_debug/control

# View debug messages
dmesg | grep anarchy
```

## Support

For additional support:
- GitHub Issues: [Project Issues](https://github.com/your-username/anarchy-egpu/issues)
- Documentation: [Project Wiki](https://github.com/your-username/anarchy-egpu/wiki)
- Community: [Discussion Forum](https://github.com/your-username/anarchy-egpu/discussions)

## Updates and Maintenance

### Updating the Module
```bash
# Stop the module
sudo modprobe -r anarchy-egpu

# Update the source
git pull

# Rebuild and reinstall
make clean
make
sudo make install

# Reload the module
sudo modprobe anarchy-egpu
```

### Maintenance Tasks
- Regularly check for updates
- Monitor system logs for errors
- Review and adjust performance profiles as needed
- Keep NVIDIA drivers up to date
- Perform periodic connection tests

## Security Considerations

1. **Module Permissions**
   - Configuration files should be owned by root
   - Module parameters should be properly restricted
   - Avoid running with unnecessary privileges

2. **Thunderbolt Security**
   - Use secure boot if available
   - Configure Thunderbolt authorization level
   - Monitor for unauthorized devices

3. **System Integration**
   - Keep system and kernel up to date
   - Monitor for security advisories
   - Follow principle of least privilege

## Additional Resources

- [Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/index.html)
- [Thunderbolt Linux Documentation](https://www.kernel.org/doc/html/latest/driver-api/thunderbolt.html)
- [NVIDIA Linux Driver Documentation](https://docs.nvidia.com/drive/drive_os_5.2.0_linux/index.html)
- [PCIe Specification](https://pcisig.com/specifications)

## License and Legal

This project is licensed under the GPL v2 license. See the LICENSE file for details.

NVIDIA and RTX are trademarks of NVIDIA Corporation.
Thunderbolt is a trademark of Intel Corporation. 