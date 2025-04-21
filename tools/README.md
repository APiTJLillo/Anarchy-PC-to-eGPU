# Anarchy eGPU Performance Monitoring Tools

This directory contains tools for monitoring and analyzing the performance of the Anarchy eGPU system.

## Performance Monitor (`perf_monitor.py`)

A real-time performance visualization tool that displays DMA throughput, latency, and Thunderbolt connection statistics.

### Features

- Real-time graphs of:
  - TX/RX throughput (MB/s)
  - DMA latency (ns)
  - Thunderbolt connection status
- Auto-scaling graphs
- Color-coded error indicators
- Current value display
- 60-second rolling window

### Requirements

- Python 3.6 or later
- `curses` module (included in standard library)
- Root access (for reading debugfs)

### Usage

```bash
# Basic usage
sudo ./perf_monitor.py

# Custom update interval
sudo ./perf_monitor.py --interval 0.5

# Custom statistics file location
sudo ./perf_monitor.py --stats-file /path/to/stats
```

### Controls

- `q`: Quit the monitor
- `r`: Reset graph scaling

### Display Elements

1. **Throughput Graphs**
   - TX throughput in MB/s (top)
   - RX throughput in MB/s (middle)
   - Auto-scaling based on maximum observed values

2. **Latency Graph**
   - DMA operation latency in nanoseconds
   - Shows trends over time

3. **Thunderbolt Status**
   - Connection count
   - Error count (highlighted in red when non-zero)

4. **Current Values**
   - Real-time display of throughput and latency

### Interpreting the Output

- **Throughput**: Higher is better. Look for consistent values without drops
- **Latency**: Lower is better. Watch for spikes or increasing trends
- **Errors**: Should remain at zero. Any increase indicates problems
- **Connections**: Should increment only during device connect/disconnect

### Troubleshooting

1. **Permission Denied**
   ```
   Error reading statistics: Permission denied
   ```
   Solution: Run with sudo or add appropriate permissions

2. **File Not Found**
   ```
   Error reading statistics: No such file or directory
   ```
   Solution: Ensure the kernel module is loaded and debugfs is mounted

3. **Display Issues**
   ```
   _curses.error: addch() returned ERR
   ```
   Solution: Increase terminal window size

## Future Tools

1. **Performance Logger**
   - Long-term performance data collection
   - CSV output for analysis
   - Coming soon

2. **Analysis Scripts**
   - Statistical analysis of performance data
   - Performance regression detection
   - Planned for future release

3. **Automated Testing**
   - Stress test scenarios
   - Performance baseline verification
   - Under development 