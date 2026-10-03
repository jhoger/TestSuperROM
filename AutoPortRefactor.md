# Auto-Port Assignment Refactor

## Overview

This document describes the refactor changes made to support automatic socket port assignment for VirtualT instances.

## Problem

Previously, test drivers had to manually specify socket ports when launching VirtualT:
- Tests hard-coded specific ports (6166, 6167, 6168, 6169, etc.)
- Different VirtualT instances needed different ports to avoid conflicts
- Tests had to be aware of which port each instance was using
- No automatic detection of available ports

## Solution

The library now supports automatic port assignment by:
1. Finding an available port before launching VirtualT
2. Using that port when starting VirtualT
3. Storing the port in the process handle for later access

## Changes

### Library Changes

#### 1. vt_process.h

Added convenience macro:
```c
#define VT_PROCESS_PORT_AUTO 0
```

Added new helper function:
```c
vt_socket_handle_t vt_process_create_socket(vt_process_handle_t handle, const char* host);
```
- Creates a socket handle connected to the process's auto-assigned port
- Automatically uses the port assigned to the process
- Convenience wrapper around `vt_socket_create()`

#### 2. vt_process.c

Modified `vt_process_launch()`:
- Determines port BEFORE launching VirtualT process
- If `config->port == 0`: finds available port, uses it for launch
- If `config->port != 0`: uses the specified port
- Stores actual port in `internal->port` for later retrieval

Modified `vt_process_find_port()`:
- Now correctly identifies AVAILABLE ports (not in-use ports)
- Tries ports 6166, 6167, 6168, 6169, 6170
- Returns 0 if no available port found

Modified `vt_process_create_process()`:
- Now takes `uint16_t port` parameter
- Uses the port when building command line: `virtualt -p <port>`

### Test Changes

All tests now use port 0 (auto-assignment):

```c
vt_process_config_t config = {
    .virtualt_path = vt_path,
    .port = 0,  /* Auto-assign port */
    .headless = true,
    .startup_timeout_ms = 30000
};

TEST_ASSERT_TRUE(vt_process_launch(process_handle, &config));

/* Get the auto-assigned port if needed */
uint16_t port = vt_process_get_port(process_handle);
```

### Port Finding Logic

When `config->port == 0`:
1. Try to connect to port 6166
2. If connection succeeds (port in use), try next port
3. If connection fails (port available), use that port
4. Repeat until available port found or all ports tried

## Usage Examples

### Basic Usage (Auto-Assignment)
```c
vt_process_handle_t process = vt_process_create();
vt_process_config_t config = {
    .virtualt_path = "/path/to/virtualt",
    .port = 0,  // Auto-assign
    .headless = true
};

if (vt_process_launch(process, &config)) {
    uint16_t port = vt_process_get_port(process);
    printf("VirtualT running on port: %d\n", port);
}
```

### Using Helper Function
```c
/* Create socket directly from process handle */
vt_socket_handle_t socket = vt_process_create_socket(process, NULL);
if (socket && vt_socket_connect(socket)) {
    vt_socket_send_command(socket, "model pc8201", response, sizeof(response));
}
```

### Explicit Port (Backward Compatible)
```c
vt_process_config_t config = {
    .virtualt_path = "/path/to/virtualt",
    .port = 6166,  // Specific port
    .headless = true
};
vt_process_launch(process, &config);
```

## Benefits

1. **No Port Conflicts**: Multiple VirtualT instances can run simultaneously without manual port management
2. **Simpler Tests**: Tests don't need to track which port each instance uses
3. **Automatic Resource Management**: Library finds available ports automatically
4. **Backward Compatible**: Explicit port specification still works
5. **Cleaner API**: `vt_process_create_socket()` hides socket details from tests

## Migration Guide

To update existing tests:

1. Change `.port = 6166` to `.port = 0`
2. Remove any manual port tracking from tests
3. Use `vt_process_get_port(process)` when port is needed
4. Optionally use `vt_process_create_socket(process, NULL)` for socket creation

## Files Modified

- `Library/vt_process/vt_process.h` - Added macro and helper function declaration
- `Library/vt_process/vt_process.c` - Implemented auto-assignment logic
- `Tests/test_vt_process.c` - Updated to use port 0
- `Tests/test_vt_virtual_t.c` - Updated to use port 0