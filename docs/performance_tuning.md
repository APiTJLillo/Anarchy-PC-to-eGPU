# Anarchy eGPU Performance Tuning Guide

This guide provides detailed information on optimizing the performance of your Anarchy eGPU setup.

## Performance Monitoring

### Using the Performance Monitor

The `perf_monitor.py` tool in the `tools` directory provides real-time performance metrics:
```bash
sudo ./tools/perf_monitor.py
```

Key metrics to watch:
- TX/RX throughput: Should be stable and near theoretical maximum
- DMA latency: Should be low and consistent
- Error count: Should remain at zero

## Optimization Areas

### 1. DMA Configuration

#### Ring Buffer Size
- Default: 256 pages
- Recommended range: 256-1024 pages
- Tuning steps:
  1. Monitor throughput with current setting
  2. Increase size in powers of 2
  3. Test with your workload
  4. Stop when performance improvement plateaus

Example:
```bash
sudo insmod anarchy-egpu.ko ring_buffer_size=512
```

#### DMA Channels
- Default: 8 channels
- Recommended range: 4-16 channels
- Considerations:
  - More channels = higher parallelism
  - Each channel consumes memory
  - Diminishing returns past 8-12 channels

### 2. PCIe Configuration

#### Link Speed
- Auto-negotiation (default) usually works best
- Force specific speed only if issues occur
- Available speeds:
  - Gen1: 2.5 GT/s
  - Gen2: 5.0 GT/s
  - Gen3: 8.0 GT/s
  - Gen4: 16.0 GT/s

#### Payload Size
- Default: 256 bytes
- Larger sizes can improve throughput
- Test with powers of 2: 256, 512, 1024
- Monitor error rates when adjusting

### 3. Thunderbolt Settings

#### Timeout Values
- Default: 1000ms
- Reduce for lower latency if stable
- Increase if seeing timeout errors
- Monitor error counts when adjusting

#### Retry Count
- Default: 3 retries
- Increase if connection is unstable
- Decrease if quick failure detection needed

## Workload-Specific Tuning

### Gaming Workloads
- Optimize for latency:
  - Lower `completion_timeout`
  - Smaller `ring_buffer_size`
  - Fewer DMA channels

### Compute Workloads
- Optimize for throughput:
  - Larger `ring_buffer_size`
  - More DMA channels
  - Larger payload sizes

### Mixed Workloads
- Balance settings:
  - Medium `ring_buffer_size` (512)
  - 8-12 DMA channels
  - Default timeout values

## Performance Troubleshooting

### High Latency
1. Check system load
2. Reduce `ring_buffer_size`
3. Decrease DMA channels
4. Monitor for system interrupts

### Low Throughput
1. Verify link speed
2. Increase `ring_buffer_size`
3. Adjust payload size
4. Check for error counts

### Connection Issues
1. Increase retry count
2. Adjust timeout values
3. Check cable quality
4. Verify power management settings

## System Considerations

### Memory Impact
- Each DMA channel: ~4MB
- Ring buffers: size * 4KB per buffer
- Monitor total memory usage

### CPU Impact
- Higher debug levels increase CPU usage
- More DMA channels increase interrupt load
- Monitor CPU usage during operation

### Power Management
- Disable power saving for best performance
- Consider thermal limitations
- Monitor GPU power state

## Best Practices

1. Baseline Performance
   - Record initial performance metrics
   - Document default settings
   - Measure key workloads

2. Incremental Changes
   - Change one parameter at a time
   - Test thoroughly after each change
   - Document improvements

3. Regular Monitoring
   - Use `perf_monitor.py` regularly
   - Watch for performance regression
   - Monitor system resources

4. Configuration Management
   - Save working configurations
   - Document workload-specific settings
   - Maintain change history 