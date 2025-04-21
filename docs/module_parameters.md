# Anarchy eGPU Kernel Module Parameters

This document describes the available parameters for the Anarchy eGPU kernel module.

## Core Parameters

### `debug_level` (int)
Controls the verbosity of debug messages.
- Default: 0
- Range: 0-3
- Values:
  - 0: Errors only
  - 1: Warnings and errors
  - 2: Info, warnings, and errors
  - 3: Debug, info, warnings, and errors

### `force_link_speed` (int)
Forces a specific PCIe link speed.
- Default: 0 (auto)
- Range: 0-4
- Values:
  - 0: Auto-negotiate
  - 1: Gen1 (2.5 GT/s)
  - 2: Gen2 (5.0 GT/s)
  - 3: Gen3 (8.0 GT/s)
  - 4: Gen4 (16.0 GT/s)

### `max_dma_channels` (int)
Maximum number of DMA channels to allocate.
- Default: 8
- Range: 1-16
- Note: Higher values allow more concurrent transfers but consume more memory

## Thunderbolt Parameters

### `tb_retry_count` (int)
Number of connection retry attempts.
- Default: 3
- Range: 1-10

### `tb_timeout_ms` (int)
Timeout for Thunderbolt operations in milliseconds.
- Default: 1000
- Range: 100-5000

### `tb_service_uuid` (string)
UUID for XDomain service identification.
- Default: "c5c7d859-5c9f-4c52-9c53-8d93b8c5c898"
- Format: Standard UUID string

## Performance Parameters

### `ring_buffer_size` (int)
Size of DMA ring buffers in pages.
- Default: 256
- Range: 16-1024
- Note: Must be a power of 2

### `max_payload_size` (int)
Maximum payload size for PCIe transactions.
- Default: 256
- Range: 128-4096
- Note: Must be a power of 2

### `completion_timeout` (int)
Timeout for DMA completion in milliseconds.
- Default: 100
- Range: 10-1000

## Example Usage

To load the module with custom parameters:
```bash
sudo insmod anarchy-egpu.ko debug_level=2 ring_buffer_size=512 tb_timeout_ms=2000
```

To modify parameters at runtime (if supported):
```bash
sudo sh -c 'echo 2 > /sys/module/anarchy_egpu/parameters/debug_level'
```

## Performance Considerations

- Increasing `ring_buffer_size` can improve throughput but consumes more memory
- Higher `debug_level` values impact performance and should be used only for troubleshooting
- `max_dma_channels` should be set based on available system resources
- `completion_timeout` may need adjustment based on system load and latency requirements

## Troubleshooting

If experiencing issues:
1. Enable debug messages with `debug_level=3`
2. Check dmesg output for detailed error information
3. Adjust timeouts if seeing completion or connection failures
4. Verify system has sufficient resources for configured buffer sizes 