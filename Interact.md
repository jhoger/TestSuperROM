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
- `vt_interact_lcd_get()` - Get LCD state (placeholder - requires memory read)

### PC-8201A Specific Operations
- `vt_interact_load_rom()` - Load the PC-8201A ROM (SUPNEC.bin)
- `vt_interact_launch_basic()` - Launch BASIC interpreter
- `vt_interact_type_line()` - Type a line of BASIC
- `vt_interact_execute_program()` - Execute current program

## Menu Navigation Functions

The library provides comprehensive menu navigation capabilities:

### vt_interact_menu_navigate()
Navigate to a specific row in the menu using arrow keys.
```c
// Navigate from current row to row 3
bool result = vt_interact_menu_navigate(handle, 3, current_row);
```

### vt_interact_launch_basic_from_menu()
Launch the BASIC interpreter by navigating the menu.
```c
// Navigate to BASIC and launch
bool result = vt_interact_launch_basic_from_menu(handle);
```

### vt_interact_exit_to_menu()
Exit back to the main menu using F8 key.
```c
// Exit from BASIC back to main menu
bool result = vt_interact_exit_to_menu(handle);
```

### vt_interact_type_program()
Type a complete BASIC program line by line.
```c
const char* program[] = {
    "10 PRINT \"HELLO\"",
    "20 FOR I = 1 TO 10",
    "30 PRINT I",
    "40 NEXT I",
    "50 PRINT \"DONE\""
};
bool result = vt_interact_type_program(handle, program, 5);
```

### vt_interact_press_key()
Press any key by name.
```c
// Press various keys
vt_interact_press_key(handle, "enter");
vt_interact_press_key(handle, "left");
vt_interact_press_key(handle, "f8");
```

### vt_interact_type_text()
Type arbitrary text with optional line ending.
```c
// Type text without newline
vt_interact_type_text(handle, "HELLO", false);

// Type text with newline
vt_interact_type_text(handle, "TEST", true);
```

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
vt_interact_launch_basic_from_menu(handle);

// Type a 10-line program
const char* program[] = {
    "10 PRINT \"HELLO WORLD\"",
    "20 FOR I = 1 TO 10",
    "30 PRINT I",
    "40 NEXT I",
    "50 PRINT \"DONE\"",
    "60 GOTO 10",
    "70 END",
    "80 REM TEST PROGRAM",
    "90 CLR",
    "100 LIST"
};
vt_interact_type_program(handle, program, 10);

// Exit back to menu
vt_interact_exit_to_menu(handle);

// Cleanup
vt_interact_terminate(handle);
vt_interact_destroy(handle);
```

## Implementation Details

### Keyboard Input
The library uses the VirtualT `key` command for all keyboard input:
```c
key "text"     // Type text
key enter      // Press ENTER
key left       // Press left arrow
key f8         // Press F8
```

### Timing Delays
The library adds appropriate delays for keystroke processing:
- 100ms after typing lines
- 50ms after arrow key presses
- 50ms after key presses

### Error Handling
All functions return `false` on error, with detailed error messages available via:
```c
const char* error = vt_interact_get_error();
printf("Error: %s\n", error);
```

## Status

The vt_interact library is fully implemented with:
- Process management functions
- Socket connection management
- System control functions
- LCD operations
- PC-8201A specific operations
- Menu navigation functions
- Complete BASIC program entry support

The library is ready for integration testing with the VirtualT emulator.