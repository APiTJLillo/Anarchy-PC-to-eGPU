# Anarchy eGPU Test Framework

This document describes the test framework for the Anarchy eGPU system, including setup, usage, and test development guidelines.

## Overview

The test framework provides comprehensive testing capabilities for:
- Unit testing of individual components
- Integration testing of system interactions
- Performance testing and benchmarking
- Error injection and recovery testing

## Test Structure

### Directory Layout
```
tests/
├── unit/
│   ├── thunderbolt/
│   ├── pcie/
│   └── dma/
├── integration/
│   ├── connection/
│   └── performance/
├── stress/
└── common/
    ├── fixtures/
    └── utils/
```

## Running Tests

### Basic Usage

Run all tests:
```bash
./run_tests.sh
```

Run specific test categories:
```bash
./run_tests.sh --unit          # Run unit tests only
./run_tests.sh --integration   # Run integration tests
./run_tests.sh --stress        # Run stress tests
```

### Test Options

```bash
./run_tests.sh [options]
  --verbose        Enable detailed output
  --quick         Skip long-running tests
  --no-cleanup    Preserve test artifacts
  --repeat N      Repeat tests N times
```

## Writing Tests

### Unit Test Example

```c
#include "test_framework.h"
#include "anarchy-egpu.h"

TEST_CASE("DMA Ring Creation") {
    struct anarchy_dma_ring *ring;
    
    SECTION("Basic Creation") {
        ring = create_dma_ring(256);
        REQUIRE(ring != NULL);
        REQUIRE(ring->size == 256);
        free_dma_ring(ring);
    }
    
    SECTION("Invalid Size") {
        ring = create_dma_ring(0);
        REQUIRE(ring == NULL);
    }
}
```

### Integration Test Example

```c
TEST_CASE("Thunderbolt Connection") {
    struct anarchy_device *dev;
    
    BEFORE_EACH() {
        dev = init_test_device();
    }
    
    AFTER_EACH() {
        cleanup_test_device(dev);
    }
    
    TEST("Connection Establishment") {
        int status = establish_connection(dev);
        REQUIRE(status == 0);
        REQUIRE(dev->connected == true);
    }
}
```

## Test Categories

### 1. Unit Tests
- Individual component functionality
- Error handling
- Edge cases
- Memory management

### 2. Integration Tests
- Component interactions
- Data flow
- State transitions
- Resource management

### 3. Performance Tests
- Throughput measurement
- Latency testing
- Resource utilization
- Scaling behavior

### 4. Stress Tests
- Long-running stability
- Resource exhaustion
- Error injection
- Recovery scenarios

## Fixtures and Utilities

### Common Test Fixtures

```c
// Device initialization fixture
struct anarchy_device *init_test_device(void) {
    struct anarchy_device *dev;
    dev = alloc_test_device();
    if (!dev) return NULL;
    setup_test_environment(dev);
    return dev;
}

// Cleanup fixture
void cleanup_test_device(struct anarchy_device *dev) {
    if (!dev) return;
    teardown_test_environment(dev);
    free_test_device(dev);
}
```

### Utility Functions

```c
// Performance measurement
uint64_t measure_throughput(struct anarchy_device *dev) {
    // Implementation
}

// Error injection
void inject_error(struct anarchy_device *dev, int error_type) {
    // Implementation
}
```

## Best Practices

### 1. Test Organization
- Group related tests together
- Use descriptive test names
- Keep tests focused and atomic
- Follow arrange-act-assert pattern

### 2. Resource Management
- Clean up resources after tests
- Use appropriate fixtures
- Handle error cases
- Check for memory leaks

### 3. Test Coverage
- Aim for high code coverage
- Test edge cases
- Include error scenarios
- Verify recovery paths

### 4. Performance Testing
- Establish baselines
- Use consistent workloads
- Account for system variation
- Document test conditions

## Debugging Failed Tests

### 1. Verbose Output
```bash
./run_tests.sh --verbose
```

### 2. Single Test Execution
```bash
./run_tests.sh --test "DMA Ring Creation"
```

### 3. Debug Logging
```bash
export ANARCHY_TEST_DEBUG=1
./run_tests.sh
```

### 4. Test Artifacts
```bash
./run_tests.sh --no-cleanup
ls tests/artifacts/
```

## Continuous Integration

### Automated Testing
```bash
# CI script example
./ci/run_tests.sh \
  --unit \
  --integration \
  --coverage \
  --report
```

### Test Reports
- Coverage analysis
- Performance metrics
- Test execution time
- Resource utilization

## Extending the Framework

### Adding New Test Types
1. Create test category directory
2. Implement test fixtures
3. Add to test runner
4. Update documentation

### Custom Assertions
```c
#define REQUIRE_THROUGHPUT(actual, expected) \
    require_within_range(actual, expected, 0.1)
```

## Troubleshooting

### Common Issues
1. Resource cleanup failures
2. Timing-sensitive tests
3. System resource limits
4. Hardware dependencies

### Solutions
1. Use cleanup fixtures
2. Add timing tolerances
3. Check system limits
4. Mock hardware interfaces 