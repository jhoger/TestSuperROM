# VirtualT Process Management Library

This document describes the vt_process library for managing the VirtualT emulator process lifecycle.

---

## Overview

The vt_process library provides functionality to launch, connect to, and tear down the VirtualT emulator process. It integrates with the vt_socket library for seamless socket connection management.

### Key Features

- **Process Management**: Launch VirtualT with configurable parameters
- **Connection Coordination**: Automatically establish socket connection after process startup
- **Graceful Shutdown**: Clean process termination with resource cleanup
- **Cross-Platform**: Support for Windows and POSIX systems
- **Timeout Protection**: Configurable timeouts for process operations



## Architecture

The vt_process library works in conjunction with vt_socket to provide complete process management:

`
Application Code
       │
    ┌──┴──┐
    │     │
vt_process  vt_socket
    │     │
    └──┬──┘
       │
 VirtualT Process
`

### Process Lifecycle States

`
[CREATED] → [LAUNCHED] → [READY] → [CONNECTED]
   │                              │
   └───────[TERMINATED] ◀─────────┘
`



## API Reference

### Types and Constants

#### Process Handle
`c
typedef struct vt_process_t* vt_process_handle_t;
`

#### Return Codes
`c
#define VT_PROCESS_SUCCESS           0
#define VT_PROCESS_ERROR            -1
#define VT_PROCESS_TIMEOUT          -2
#define VT_PROCESS_ALREADY_STARTED  -3
#define VT_PROCESS_NOT_STARTED      -4
#define VT_PROCESS_CONNECTION_FAIL  -5
`

#### Configuration Structure
`c
typedef struct {
    const char* virtualt_path;    // Path to VirtualT executable
    uint16_t    port;             // Socket port (0 = auto-assign)
    const char* rom_path;         // Optional ROM file path
    bool        headless;         // Run without GUI
    uint32_t    startup_timeout_ms; // Wait time for process ready
} vt_process_config_t;
`

#### Process Status
`c
typedef struct {
    bool is_running;              // Process is active
    bool is_connected;            // Socket connection established
    uint16_t port;                // Active socket port
    pid_t pid;                    // Process ID
} vt_process_status_t;
`



### Core Functions

#### vt_process_create

Creates a new process handle.

`c
vt_process_handle_t vt_process_create(void);
`

**Returns:**
- Process handle on success
- NULL if allocation failed

---

#### vt_process_launch

Launches the VirtualT process with the given configuration.

`c
bool vt_process_launch(vt_process_handle_t handle, 
                       const vt_process_config_t* config);
`

**Parameters:**
- handle - Process handle created by t_process_create
- config - Process configuration

**Returns:**
- 	rue on success, alse on failure

**Notes:**
- On Windows, uses CreateProcess with proper path handling
- On POSIX systems, uses ork/exec with xecvp
- The process will be launched with the specified port

---

#### vt_process_wait_for_ready

Waits for the VirtualT process to be ready to accept connections.

`c
bool vt_process_wait_for_ready(vt_process_handle_t handle, 
                               uint32_t timeout_ms);
`

**Parameters:**
- handle - Process handle
- 	imeout_ms - Maximum wait time in milliseconds (0 = use default)

**Returns:**
- 	rue if process is ready within timeout
- alse if timeout expired or process failed to start

**Notes:**
- Attempts to connect to the socket to verify readiness
- Can be called multiple times with different timeouts

---

#### vt_process_connect

Establishes a socket connection to the VirtualT process.

`c
bool vt_process_connect(vt_process_handle_t handle, 
                        vt_socket_handle_t socket_handle);
`

**Parameters:**
- handle - Process handle
- socket_handle - vt_socket handle for connection

**Returns:**
- 	rue on successful connection
- alse if connection failed

**Notes:**
- Uses the vt_socket library for connection
- The socket handle must be created before calling this

---

#### vt_process_disconnect

Closes the socket connection to VirtualT.

`c
void vt_process_disconnect(vt_process_handle_t handle);
`

**Parameters:**
- handle - Process handle

---

#### vt_process_terminate

Terminates the VirtualT process gracefully.

`c
bool vt_process_terminate(vt_process_handle_t handle);
`

**Parameters:**
- handle - Process handle

**Returns:**
- 	rue on successful termination
- alse if process was not running or termination failed

**Notes:**
- Sends termination command to VirtualT via socket if connected
- Falls back to process kill if socket is unavailable
- Cleans up all associated resources

---

#### vt_process_destroy

Cleans up the process handle and releases resources.

`c
void vt_process_destroy(vt_process_handle_t handle);
`

**Parameters:**
- handle - Process handle to destroy



### Status Functions

#### vt_process_get_status

Gets the current status of the VirtualT process.

`c
bool vt_process_get_status(vt_process_handle_t handle, 
                           vt_process_status_t* status);
`

**Parameters:**
- handle - Process handle
- status - Pointer to status structure to fill

**Returns:**
- 	rue if status was retrieved
- alse if handle is invalid

---

#### vt_process_get_port

Gets the port number the VirtualT process is using.

`c
uint16_t vt_process_get_port(vt_process_handle_t handle);
`

**Parameters:**
- handle - Process handle

**Returns:**
- Port number if process is running
- 0 if process is not running

---

#### vt_process_is_running

Checks if the VirtualT process is currently running.

`c
bool vt_process_is_running(vt_process_handle_t handle);
`

**Parameters:**
- handle - Process handle

**Returns:**
- 	rue if process is running
- alse if process is not running



## Usage Examples

### Basic Process Lifecycle

`c
#include \
vt_process.h\
#include \vt_socket.h\

int main(void) {
    // Create process handle
    vt_process_handle_t process = vt_process_create();
    if (!process) {
        return -1;
    }
    
    // Configure and launch
    vt_process_config_t config = {
        .virtualt_path = \C:\\\\VirtualT\\\\virtualt.exe\,
        .port = 0,  // Auto-assign
        .headless = false,
        .startup_timeout_ms = 10000
    };
    
    if (!vt_process_launch(process, &config)) {
        printf(\Failed
to
launch
VirtualT\\n\);
        vt_process_destroy(process);
        return -1;
    }
    
    // Wait for process to be ready
    if (!vt_process_wait_for_ready(process, 0)) {
        printf(\Process
failed
to
start\\n\);
        vt_process_terminate(process);
        vt_process_destroy(process);
        return -1;
    }
    
    // Get the assigned port
    uint16_t port = vt_process_get_port(process);
    printf(\VirtualT
running
on
port
%d\\n\, port);
    
    // Create and connect socket
    vt_socket_handle_t socket = vt_socket_create(\127.0.0.1\, port);
    if (!vt_process_connect(process, socket)) {
        printf(\Failed
to
connect
to
VirtualT\\n\);
        vt_process_terminate(process);
        vt_process_destroy(process);
        return -1;
    }
    
    // Use the socket...
    vt_cpu_halt(socket);
    
    // Cleanup
    vt_socket_disconnect(socket);
    vt_socket_destroy(socket);
    vt_process_disconnect(process);
    vt_process_terminate(process);
    vt_process_destroy(process);
    
    return 0;
}
`



### With Error Handling

`c
bool run_virtualt_test(void) {
    vt_process_handle_t process = vt_process_create();
    if (!process) {
        return false;
    }
    
    // Set up event callback
    vt_process_set_callback(process, my_event_callback, NULL);
    
    vt_process_config_t config = {
        .virtualt_path = get_virtualt_path(),
        .port = 6166,
        .rom_path = \
test.rom\,
        .headless = true,
        .startup_timeout_ms = 15000
    };
    
    if (!vt_process_launch(process, &config)) {
        fprintf(stderr, \Error:
Could
not
launch
VirtualT\\n\);
        vt_process_destroy(process);
        return false;
    }
    
    // Wait with timeout
    for (int attempts = 0; attempts < 5; attempts++) {
        if (vt_process_wait_for_ready(process, 2000)) {
            break;
        }
        if (attempts == 4) {
            fprintf(stderr, \Error:
Process
not
ready
after
10
seconds\\n\);
            vt_process_terminate(process);
            vt_process_destroy(process);
            return false;
        }
    }
    
    // Continue with testing...
    
    vt_process_terminate(process);
    vt_process_destroy(process);
    return true;
}
`

## Platform-Specific Notes

### Windows

- VirtualT executable should be in PATH or fully specified
- Firewall may need to be configured for the socket port
- Process termination uses TerminateProcess on Windows

### POSIX (Linux/macOS)

- Use fork/exec for process spawning
- Signal handling for SIGTERM/SIGINT
- May need to set up proper PATH for VirtualT

### Path Configuration

The VIRTUALT_PATH should point to the VirtualT home directory:
It must be set in the shell environment before launching the test driver. The working directory must be set to the directory as well.

VirtualT.exe will reside in that directory on windows.

In the code, the path is held in virtualt_path but that path should never be hardcoded.

Always compute the fullpath using the environment variable concatenated with the executable name.
On windows the name is VirtualT.exe
On Linux the name is virtualt

If the environment variable is not set, the test driver should exit with an error with instructions to set the environment variable.

## Error Codes

| Code | Description |
|------|-------------|
| VT_PROCESS_SUCCESS | Operation completed successfully |
| VT_PROCESS_ERROR | Generic error |
| VT_PROCESS_TIMEOUT | Operation timed out |
| VT_PROCESS_ALREADY_STARTED | Process already running |
| VT_PROCESS_NOT_STARTED | Process not started |
| VT_PROCESS_CONNECTION_FAIL | Socket connection failed |

## Implementation Details

### Process State Machine

`
[UNINITIALIZED] 
       │
       ▼
[CREATED] ──launch()──▶ [LAUNCHED] ──wait_for_ready()──▶ [READY]
       │                                      │
       │                                      ▼
       │                             [CONNECTED] ──disconnect()──▶ [READY]
       │                                      │
       │                                      ▼
       │                             [TERMINATED] ◀── terminate()
       │                                      │
       └──────────────────────────────────────┘
                   destroy()
`

### Resource Cleanup

When vt_process_terminate is called:
1. If connected via socket, sends 	erminate command to VirtualT
2. Closes socket connection
3. Terminates the process
4. Cleans up any allocated memory

When vt_process_destroy is called:
1. Calls vt_process_terminate if process is running
2. Frees the process handle
3. Clears all internal state

## Testing

The library includes unit tests covering:
- Process launch and termination
- Socket connection establishment
- Error handling scenarios
- Timeout behavior
- State transitions

Run tests with:
`bash
cmake --build build --target vt_process_tests
`

## See Also

- [Socket.md](Socket.md) - VirtualT socket interface documentation
- [vt_socket.h](../Library/vt_socket/vt_socket.h) - Socket library header

