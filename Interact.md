# vt_interact Library - High-Level PC-8201A Control

The vt_interact library provides higher-level functions for interacting with VirtualT
emulated computers, specifically the NEC PC-8201A. It builds on top of vt_socket and
vt_process to provide convenient operations.

## Core Functions

### Process Management
- `vt_interact_create()` - Create handle for VirtualT interaction
- `vt_interact_destroy()` - Clean up and release resources
- `vt_interact_launch()` - Launch VirtualT with PC-8201A configuration
- `vt_interact_wait_for_ready()` - Wait for VirtualT to be ready
- `vt_interact_terminate()` - Gracefully terminate VirtualT

### Socket Connection
- `vt_interact_connect()` - Connect to VirtualT socket
- `vt_interact_disconnect()` - Disconnect from VirtualT socket
- `vt_interact_is_connected()` - Check connection status

### System Control
- `vt_interact_set_model()` - Set emulated model (pc8201, m100, m200)
- `vt_interact_get_status()` - Get system status
- `vt_interact_halt_cpu()` - Halt CPU execution
- `vt_interact_run_cpu()` - Resume CPU execution

### LCD Operations
- `vt_interact_lcd_clear()` - Clear LCD display
- `vt_interact_lcd_write()` - Write text to LCD at position
- `vt_interact_lcd_get()` - Get LCD state (placeholder)

### PC-8201A Specific Operations
- `vt_interact_load_rom()` - Load the PC-8201A ROM (SUPNEC.bin)
- `vt_interact_launch_basic()` - Launch BASIC interpreter
- `vt_interact_type_line()` - Type a line of BASIC
- `vt_interact_execute_program()` - Execute current program

## Key Discovery

The `key` command (from VirtualT socket interface) enables full keyboard input:
- Quoted strings: `"text"` types the text
- Special keys: `enter`, `left`, `right`, `up`, `down`, `graph`, `ctrl`, etc.
- Sequences: `right enter "file.do" f8`

This allows full menu navigation and BASIC program entry.

## Usage Example

```c
vt_interact_handle_t handle = vt_interact_create();
vt_interact_config_t config = {
    .virtualt_path = vt_path,
    .port = 0,  // Auto-assign
    .headless = true
};

vt_interact_launch(handle, &config);
vt_interact_wait_for_ready(handle, 30000);
vt_interact_connect(handle);

// Set model to PC-8201A
vt_interact_set_model(handle, "pc8201");

// Load ROM
vt_interact_load_rom(handle);

// Navigate menu and launch BASIC
vt_interact_type_line(handle, "key right right enter");

// Type BASIC program
vt_interact_type_line(handle, "10 PRINT \"HELLO\"");
vt_interact_type_line(handle, "20 GOTO 10");

// Execute program
vt_interact_execute_program(handle);

// Cleanup
vt_interact_terminate(handle);
vt_interact_destroy(handle);
```

## Status

The vt_interact library is in development. The core infrastructure is in place and
ready for expansion with the complete command set from the VirtualT socket interface.