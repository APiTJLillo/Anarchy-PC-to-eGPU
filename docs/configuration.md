# Anarchy eGPU Configuration Reference

## Overview

The Anarchy eGPU system uses a JSON-based configuration system with three main components:
1. Main configuration file (`config.json`)
2. Performance profiles
3. Monitoring thresholds

All configuration files are stored in `/etc/anarchy-egpu/`.

## Main Configuration File

### Location
```
/etc/anarchy-egpu/config.json
```

### Structure
```json
{
    "preferences": {
        "auto_connect": true,
        "auto_power_management": true,
        "persistent_monitoring": true,
        "monitoring_interval": 1000,
        "history_retention": 24,
        "default_profile": "balanced",
        "notifications_enabled": true,
        "log_level": 3
    },
    "thresholds": {
        "temp_warning": 80,
        "temp_critical": 90,
        "power_warning": 300,
        "power_critical": 350,
        "memory_warning": 90,
        "memory_critical": 95,
        "fan_speed_warning": 80,
        "fan_speed_critical": 90,
        "pcie_error_threshold": 10,
        "dma_error_threshold": 5
    }
}
```

### Preferences Settings

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| auto_connect | boolean | true | Automatically connect to GPU when detected |
| auto_power_management | boolean | true | Enable automatic power management |
| persistent_monitoring | boolean | true | Keep monitoring history across sessions |
| monitoring_interval | integer | 1000 | Monitoring update interval in milliseconds |
| history_retention | integer | 24 | Hours to retain monitoring history |
| default_profile | string | "balanced" | Default performance profile name |
| notifications_enabled | boolean | true | Enable system notifications |
| log_level | integer | 3 | Logging verbosity (0=ERROR, 1=WARN, 2=INFO, 3=DEBUG) |

### Threshold Settings

| Setting | Type | Default | Unit | Description |
|---------|------|---------|------|-------------|
| temp_warning | integer | 80 | °C | Temperature warning threshold |
| temp_critical | integer | 90 | °C | Temperature critical threshold |
| power_warning | integer | 300 | W | Power consumption warning threshold |
| power_critical | integer | 350 | W | Power consumption critical threshold |
| memory_warning | integer | 90 | % | Memory usage warning threshold |
| memory_critical | integer | 95 | % | Memory usage critical threshold |
| fan_speed_warning | integer | 80 | % | Fan speed warning threshold |
| fan_speed_critical | integer | 90 | % | Fan speed critical threshold |
| pcie_error_threshold | integer | 10 | count | PCIe error count threshold |
| dma_error_threshold | integer | 5 | count | DMA error count threshold |

## Performance Profiles

### Location
```
/etc/anarchy-egpu/profiles/
```

### Profile Structure
```json
{
    "name": "profile_name",
    "power_limit": 300,
    "temp_target": 80,
    "fan_speed_min": 40,
    "fan_speed_max": 80,
    "core_clock_offset": 0,
    "mem_clock_offset": 0,
    "adaptive_power": true
}
```

### Profile Settings

| Setting | Type | Range | Unit | Description |
|---------|------|-------|------|-------------|
| name | string | - | - | Profile identifier |
| power_limit | integer | 100-450 | W | GPU power limit |
| temp_target | integer | 60-90 | °C | Target temperature |
| fan_speed_min | integer | 0-100 | % | Minimum fan speed |
| fan_speed_max | integer | 0-100 | % | Maximum fan speed |
| core_clock_offset | integer | -500 to +500 | MHz | GPU core clock offset |
| mem_clock_offset | integer | -1000 to +1000 | MHz | Memory clock offset |
| adaptive_power | boolean | - | - | Enable adaptive power management |

### Default Profiles

1. **power_saver.json**
```json
{
    "name": "power_saver",
    "power_limit": 250,
    "temp_target": 75,
    "fan_speed_min": 30,
    "fan_speed_max": 70,
    "core_clock_offset": -100,
    "mem_clock_offset": 0,
    "adaptive_power": true
}
```

2. **balanced.json**
```json
{
    "name": "balanced",
    "power_limit": 300,
    "temp_target": 80,
    "fan_speed_min": 40,
    "fan_speed_max": 80,
    "core_clock_offset": 0,
    "mem_clock_offset": 0,
    "adaptive_power": true
}
```

3. **performance.json**
```json
{
    "name": "performance",
    "power_limit": 350,
    "temp_target": 85,
    "fan_speed_min": 50,
    "fan_speed_max": 100,
    "core_clock_offset": 100,
    "mem_clock_offset": 200,
    "adaptive_power": false
}
```

## Module Parameters

### Available Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| auto_connect | bool | 1 | Auto-connect on module load |
| debug | bool | 0 | Enable debug output |
| force_rescan | bool | 0 | Force PCIe bus rescan |
| power_limit | int | 300 | Default power limit (W) |
| profile | string | "balanced" | Default profile name |

### Setting Parameters

Via modprobe:
```bash
modprobe anarchy-egpu auto_connect=0 debug=1
```

Via configuration file:
```bash
echo "options anarchy-egpu auto_connect=0 debug=1" > /etc/modprobe.d/anarchy-egpu.conf
```

## Monitoring and Statistics

### Available Metrics

| Metric | Type | Unit | Description |
|--------|------|------|-------------|
| gpu_temp | integer | °C | GPU core temperature |
| gpu_power | integer | W | GPU power consumption |
| gpu_util | integer | % | GPU utilization |
| mem_util | integer | % | Memory utilization |
| fan_speed | integer | % | Fan speed percentage |
| core_clock | integer | MHz | Current core clock |
| mem_clock | integer | MHz | Current memory clock |
| pcie_errors | integer | count | PCIe error count |
| dma_errors | integer | count | DMA error count |

### Monitoring Configuration

The monitoring system can be configured through the main configuration file:

```json
{
    "monitoring": {
        "enabled": true,
        "interval": 1000,
        "history_size": 3600,
        "log_to_file": true,
        "log_path": "/var/log/anarchy-egpu/monitoring.log"
    }
}
```

## Error Handling

### Error Types

| Error Code | Description | Recovery Action |
|------------|-------------|-----------------|
| TIMEOUT | Operation timeout | Retry operation |
| ECC | Memory ECC error | Reset GPU |
| THERMAL | Temperature exceeded | Reduce power/clocks |
| POWER | Power limit exceeded | Reduce power limit |
| MEMORY | Memory access error | Reset memory controller |
| ENGINE | GPU engine fault | Reset engine |
| FATAL | Unrecoverable error | Full GPU reset |

### Error Recovery

The system implements automatic error recovery with the following steps:

1. Error detection
2. Error classification
3. Recovery attempt
4. Verification
5. Fallback actions

Recovery settings can be configured in the main configuration file:

```json
{
    "error_handling": {
        "max_retries": 3,
        "retry_delay": 1000,
        "auto_recovery": true,
        "fallback_profile": "power_saver"
    }
}
```

## Security

### File Permissions

| File/Directory | Owner | Permissions | Description |
|----------------|-------|-------------|-------------|
| /etc/anarchy-egpu/ | root:root | 755 | Configuration directory |
| config.json | root:root | 644 | Main configuration |
| profiles/ | root:root | 755 | Profiles directory |
| *.json | root:root | 644 | Profile files |

### Access Control

1. Only root can modify configuration files
2. Regular users can read configurations
3. Module parameters restricted to root
4. Monitoring data accessible to all users

## Best Practices

1. **Configuration Management**
   - Keep backup of working configurations
   - Test changes in safe mode first
   - Document custom configurations
   - Use version control for profiles

2. **Performance Tuning**
   - Start with conservative settings
   - Test stability before increasing limits
   - Monitor temperatures carefully
   - Keep safety margins in thresholds

3. **Maintenance**
   - Regular configuration review
   - Update threshold values as needed
   - Clean up old monitoring data
   - Verify file permissions

4. **Troubleshooting**
   - Enable debug logging when needed
   - Check system logs for errors
   - Verify configuration syntax
   - Test with default settings first 