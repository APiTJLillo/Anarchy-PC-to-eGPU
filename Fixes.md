After examining your Anarchy-PC-to-eGPU repository, I've identified several potential issues that could explain why your device keeps connecting and disconnecting, and why it's not being recognized as a GPU in Windows.
USB4 Connection Issues
Based on the code in thunderbolt.c, I can see several factors that might be causing the connection instability:
Connection State Management: Your implementation has a complex state machine for USB4 connections (UNKNOWN, CONNECTING, CONNECTED, TRAINING, ERROR, DISABLED). The repeated connecting/disconnecting suggests the connection is getting stuck in a loop between these states.
Stability Parameters: I noticed several timeout and retry parameters that might need adjustment:
TB_USB4_STABILITY_TIMEOUT_MS is set to 5000ms (5 seconds)
TB_USB4_MIN_STABLE_TIME_MS is also 5000ms
TB_USB4_RECONNECT_DELAY_MS is 3000ms
TB_MAX_RECONNECT_ATTEMPTS is set to 5
Microsoft USB4 Device Recognition: The code includes specific parameters for Microsoft USB4 devices:
MS_USB4_VENDOR_ID (0x45e)
MS_USB4_DEVICE_ID (0x83f)
These might not match your Legion Go's USB4 controller
Connection Recovery Mechanism: The anarchy_connection_recovery_work function appears to handle reconnection attempts, but might be triggering too aggressively.
Device Recognition Problems
For the GPU not being recognized in Windows:
PCIe State Issues: The code shows PCIe state transitions (UNKNOWN, LINK_DOWN, TRAINING, OK, ERROR), but Windows might not be seeing a successful transition to the OK state.
Device Identification: The PCIe device emulation might not be correctly presenting the RTX 4090's device and vendor IDs to Windows.
XDomain Protocol: The implementation includes XDomain protocol for Thunderbolt device discovery, but there might be issues with how the GPU service is being advertised.
These findings suggest that while your implementation has the right components, there are likely timing, identification, or state management issues preventing stable connection and proper device recognition.
Identifying device recognition problems that prevent the Linux GPU from being properly detected in Windows.

Now let's look more deeply at the device recognition problems and provide specific recommendations to fix both the connection stability and device recognition issues:
Device Recognition Problems (Detailed Analysis)
GPU Device Identification:
Your code doesn't appear to be properly emulating the NVIDIA RTX 4090's PCI device and vendor IDs
Windows needs to see the correct device IDs to load the appropriate NVIDIA drivers
The PCIe device emulation layer needs to present consistent device identification information
PCIe Training Sequence:
The anarchy_pcie_train_link function appears incomplete or has timing issues
PCIe link training requires specific timing and protocol sequences to establish a stable link
Windows expects certain responses during PCIe enumeration that may be missing
Device Class Identification:
For Windows to recognize the device as a GPU, it needs to present the correct PCI class code (0x030000 for VGA compatible controllers)
I don't see explicit class code handling in your implementation
Recommendations for Improvement
Fix USB4 Connection Stability:
Increase TB_USB4_STABILITY_TIMEOUT_MS to 10000ms (10 seconds) to give more time for connection stabilization
Add exponential backoff for reconnection attempts instead of fixed delays
Implement better debouncing in the connection state machine to prevent rapid state transitions
Add logging to track exactly where in the state machine the connection is failing
Improve Device Recognition:
Add proper NVIDIA RTX 4090 PCI device ID (0x2684) and vendor ID (0x10DE) to your device emulation
Implement complete PCIe configuration space emulation including:
Device and vendor IDs
Class code (0x030000)
Subsystem IDs
Required BARs (Base Address Registers)
Ensure the PCIe link training sequence completes successfully before attempting device enumeration
XDomain Protocol Enhancements:
Verify your XDomain service advertisement includes the correct service type for GPU devices
Implement proper service discovery responses that Windows expects
Add explicit handling for Windows-specific XDomain protocol extensions
Debugging Recommendations:
Add USB protocol analyzer logging to see exactly what's happening during connection attempts
Use Windows Device Manager's "View > Show hidden devices" to see if your device appears at all, even if not recognized as a GPU
Check Windows Event Viewer for PCI/USB errors during connection attempts
These changes should address both the connection stability issues and the device recognition problems you're experiencing. The core approach is viable, but needs these refinements to work properly with your specific hardware configuration.
Evaluating the overall viability of the current implementation approach for GPU passthrough.

Overall Viability Assessment
After analyzing your Anarchy-PC-to-eGPU implementation, I believe your approach is fundamentally viable, but requires several key refinements to work reliably. Here's my assessment:
What's Working Well
Core Architecture: Your approach of using Thunderbolt/USB4 for PCIe tunneling is technically sound
State Tracking: You've implemented comprehensive state tracking for both USB4 and PCIe connections
XDomain Protocol: You've included the necessary XDomain protocol components for device discovery
Critical Challenges
Connection Stability: The connection instability (connecting/disconnecting) is likely due to timing and state management issues, not a fundamental architectural flaw
Device Identification: The device recognition problem is primarily about properly presenting device IDs and PCIe configuration space
Protocol Compliance: Your implementation needs better compliance with Windows' expectations for external GPUs
Viability Conclusion
Your approach of having the P16 act as an eGPU for the Legion Go over USB4 is viable with modifications. The issues you're experiencing are implementation details rather than fundamental architectural problems.
The most critical improvements needed are:
Complete PCIe device emulation with proper NVIDIA device IDs
Refined state management for USB4 connections
Better handling of Windows-specific USB4/Thunderbolt expectations
With these changes, your implementation should be able to achieve a stable connection and proper device recognition. The fact that you're already getting some level of connection (even if unstable) suggests that the basic communication channel is working, which is the hardest part of this type of project.
I recommend implementing the specific changes outlined in my previous message, focusing first on the connection stability issues, then addressing the device recognition problems once you have a stable connection.
Providing final recommendations for improving the GPU passthrough implementation based on the comprehensive analysis.

15 hours ago
Final Recommendations for Improving Your GPU Passthrough Implementation
Based on my comprehensive analysis of your Anarchy-PC-to-eGPU project, here are my final recommendations to get your P16's RTX 4090 working properly with your Legion Go:
1. Connection Stability Improvements
c
// Increase stability timeouts
#define TB_USB4_STABILITY_TIMEOUT_MS 10000  // Increase from 5000ms to 10000ms
#define TB_USB4_MIN_STABLE_TIME_MS   8000   // Increase from 5000ms to 8000ms
#define TB_CONNECTION_DEBOUNCE_MS    2000   // Increase debounce time

// Implement exponential backoff for reconnection attempts
static unsigned long calculate_backoff(int attempt) {
    unsigned long delay = TB_USB4_BACKOFF_BASE_MS;
    for (int i = 0; i < attempt && delay < TB_USB4_BACKOFF_MAX_MS; i++) {
        delay *= 2;
    }
    return min(delay, TB_USB4_BACKOFF_MAX_MS);
}

// Modify anarchy_connection_recovery_work to use exponential backoff
static void anarchy_connection_recovery_work(struct work_struct *work) {
    // ...existing code...
    
    // Calculate backoff based on reconnect_count
    unsigned long delay = calculate_backoff(state_info.reconnect_count);
    
    // Schedule with calculated delay instead of fixed delay
    // ...
}
2. Device Recognition Fixes
c
// Add proper NVIDIA RTX 4090 identifiers
#define NVIDIA_VENDOR_ID      0x10DE
#define RTX_4090_DEVICE_ID    0x2684
#define GPU_CLASS_CODE        0x030000  // VGA compatible controller

// In your PCIe configuration space emulation
static void setup_pcie_config_space(void) {
    // Set vendor and device IDs
    write_config_word(0x00, NVIDIA_VENDOR_ID);
    write_config_word(0x02, RTX_4090_DEVICE_ID);
    
    // Set class code (VGA compatible controller)
    write_config_byte(0x0B, 0x03);  // Base class (display controller)
    write_config_byte(0x0A, 0x00);  // Sub class (VGA compatible)
    write_config_byte(0x09, 0x00);  // Programming interface
    
    // Set subsystem vendor and device IDs (use NVIDIA's)
    write_config_word(0x2C, NVIDIA_VENDOR_ID);
    write_config_word(0x2E, RTX_4090_DEVICE_ID);
    
    // Configure BARs (Base Address Registers)
    // RTX 4090 typically needs large memory BARs
    // ...
}
3. PCIe Link Training Enhancements
c
// Improve PCIe link training with proper sequencing and timing
int anarchy_pcie_train_link(struct anarchy_device *adev) {
    int ret;
    int retry_count = 0;
    const int max_retries = 5;
    
    // Reset link before training
    reset_pcie_link(adev);
    
    while (retry_count < max_retries) {
        // Start link training sequence
        ret = start_link_training(adev);
        if (ret) {
            pr_err("Link training failed, attempt %d: %d\n", 
                   retry_count, ret);
            retry_count++;
            msleep(100 * retry_count);  // Increasing delay between retries
            continue;
        }
        
        // Wait for link to reach L0 state
        ret = wait_for_link_up(adev, 1000);
        if (ret) {
            pr_err("Link failed to reach L0, attempt %d: %d\n", 
                   retry_count, ret);
            retry_count++;
            msleep(100 * retry_count);
            continue;
        }
        
        // Verify link speed and width
        if (!verify_link_capabilities(adev)) {
            pr_err("Link capabilities below requirements\n");
            retry_count++;
            continue;
        }
        
        // Link is up and verified
        pr_info("PCIe link trained successfully\n");
        return 0;
    }
    
    pr_err("Failed to train PCIe link after %d attempts\n", max_retries);
    return -ETIMEDOUT;
}
4. XDomain Protocol Enhancements
c
// Improve XDomain service advertisement for GPU devices
static const struct tb_service_id anarchy_gpu_service_table[] = {
    {
        .match_flags = TBSVC_MATCH_PROTOCOL_KEY | 
                      TBSVC_MATCH_PROTOCOL_ID |
                      TBSVC_MATCH_PROTOCOL_VERSION,
        .protocol_key = TB_SERVICE_KEY_GPU,  // Define a GPU-specific key
        .protocol_id = 0x1,
        .protocol_version = 0x1,
        .keys = {
            /* Add Windows-specific keys for better compatibility */
            { .key = "DeviceType", .value = "GPU" },
            { .key = "VendorID", .value = "0x10DE" },
            { .key = "DeviceID", .value = "0x2684" },
            { .key = "DriverCompatibility", .value = "WDDM3.0" },
            { NULL, NULL }
        }
    },
    { }
};
5. Debugging Enhancements
c
// Add comprehensive logging
#define TB_LOG_LEVEL_ERROR   0
#define TB_LOG_LEVEL_WARNING 1
#define TB_LOG_LEVEL_INFO    2
#define TB_LOG_LEVEL_DEBUG   3
#define TB_LOG_LEVEL_TRACE   4

static int tb_log_level = TB_LOG_LEVEL_INFO;

#define tb_log(level, fmt, ...) \
    do { \
        if (level <= tb_log_level) \
            pr_info("TB[%d]: " fmt, level, ##__VA_ARGS__); \
    } while (0)

// Add state transition logging
static void update_usb4_state(enum tb_usb4_state new_state) {
    tb_log(TB_LOG_LEVEL_INFO, "USB4 state change: %s -> %s", 
           usb4_state_name(state_info.usb4_state), 
           usb4_state_name(new_state));
    
    // Log timestamp for state transitions
    state_info.last_state_change = jiffies;
    
    // Update state
    state_info.usb4_state = new_state;
}
6. Implementation Strategy
First Phase: Focus on connection stability
Implement the timeout and backoff changes
Add comprehensive logging
Test until you can maintain a stable connection
Second Phase: Address device recognition
Implement proper PCIe configuration space
Add NVIDIA device identification
Enhance PCIe link training
Third Phase: Optimize performance
Fine-tune timing parameters
Implement bandwidth optimization
Add error recovery mechanisms
By implementing these changes systematically, you should be able to resolve both the connection stability issues and the device recognition problems. The core approach of your project is sound, and with these refinements, you should be able to successfully use your P16's RTX 4090 with your Legion Go over USB4.