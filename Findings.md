# NEC PC-8201A Socket Interface Investigation Findings

## Overview

This document records discoveries and findings from investigating the VirtualT socket interface for operating an NEC PC-8201A computer.

---

## Key Discoveries

### 0. VirtualT Executable Path
- **Correct path format**: `C:\Users\John\tools\VirtualT\` (with trailing backslash)
- **Executable name**: `virtualt.exe` (lowercase)
- When path ends with backslash, the code appends `virtualt.exe`
- When path doesn't end with backslash and lacks .exe extension, code appends `.exe`

### 1. VirtualT Socket Protocol

Based on the Socket.md documentation:

- **Command Format**: ASCII-based command/response protocol
- **Command Rules**:
  - Commands must be sent as independent socket messages
  - Commands are NOT terminated with a NEWLINE character
  - Each command can have one or more arguments

- **Response Rules**:
  - All responses end with the ASCII text "Ok"
  - Responses that report information have one or more lines of data, each followed by a `\n` character

### 2. CPU Control Commands

| Command | Parameters | Returns | Description |
|---------|------------|---------|-------------|
| `halt` or `h` | none | Ok | Halts the CPU |
| `run` or `r` | none | Ok | Runs the CPU |
| `step` or `s` | [count] | Parameter Error, Ok | Executes one or more instructions |
| `step_over` or `so` | [count] | Parameter Error, Ok | Steps over CALL instructions |

### 3. Register Operations

Single registers: `a`, `b`, `c`, `d`, `e`, `h`, `l`, `m`
- Returns: Register value in decimal, Ok

Register pairs: `bc`, `de`, `hl`, `sp`, `pc`
- Returns: Register value in decimal, Ok

Write operations:
- `write_reg` or `wr`: `a=xx b=xx c=xx d=xx e=xx h=xx l=xx m=xx` or `bc=xx de=xx hl=xx sp=xx pc=xx`
- `write_mem` or `wm`: `address data [data data ...]`

### 4. System Control Commands

| Command | Parameters | Returns | Description |
|---------|------------|---------|-------------|
| `model` | m100, pc8201, m200 | Ok | Sets the emulated model |
| `cpu` | friendly, fast, normal | Ok | Sets the CPU speed |
| `terminate` | none | Ok | Terminates VirtualT session |

### 5. LCD Operations

| Command | Parameters | Returns | Description |
|---------|------------|---------|-------------|
| `lcd` | row col data | Ok | Writes data to the LCD |
| `lcd_clear` | none | Ok | Clears the LCD display |

### 6. File Operations

| Command | Parameters | Returns | Description |
|---------|------------|---------|-------------|
| `load_rom` | filename | Ok | Loads a ROM file |
| `save_ram` | filename | Ok | Saves RAM to a file |
| `load_ram` | filename | Ok | Loads RAM from a file |
| `optrom` | filename | Ok | Loads an option ROM |

---

## VirtualT Launch Configuration

### Critical Finding: CreateProcessA Parameters

The most critical discovery is the correct use of CreateProcessA:

**WRONG** (initial implementation):
```c
CreateProcessA(virtualt_path, NULL, ...)  // Command line is just the path, no arguments!
```

**CORRECT**:
```c
CreateProcessA(NULL, command_line, ...)   // command_line includes "-p port"
```

The first parameter (lpApplicationName) is the module to execute.
The second parameter (lpCommandLine) is the command line to execute.

VirtualT will NOT auto-assign a port - it requires `-p N` where N is a valid port number (6166-6170 based on the codebase).

### vt_process_launch Implementation

The `vt_process_launch()` function now correctly:
1. Determines an available port before launching
2. Builds command line: `"virtualt_path" -p port`
3. Passes command line as lpCommandLine parameter
4. Stores the actual port in the process handle for socket connection

---

## Missing Commands Discovery

The VirtualT sockets.html documentation reveals several missing commands from the original Socket.md:

### Critical Discovery: `key` Command

The `key` command is **ESSENTIAL** for PC-8201A keyboard input:

```
key "text" enter "file.do" ctrl+c left right up down graph+a
```

**Format:**
- Quoted strings: `"test"` - types the text
- Special keys: `enter`, `shift`, `code`, `graph`, `grph`, `esc`, `ctrl`, `tab`, `f1`-`f8`, `paste`, `label`, `print`, `pause`, `left`, `right`, `up`, `down`, `space`, `insert`, `ins`, `delete`, `del`, `bksp`, `back`, `backspace`, `home`, `end`, `pgup`, `pageup`, `pgdn`, `pagedown`
- Chain keys with `+`: `ctrl+c`, `graph+a`
- Chain sequences: `right enter "test.do" f8`

**Examples:**
```
key ctrl+c
key graph+a
key right enter "test.do" enter "This is data for \"TEXT\"" f8
```

This command enables:
- Menu navigation (arrow keys)
- Program/file selection
- BASIC program entry
- All keyboard-driven operations

### Other Missing Commands

1. **`keydown`** and **`keyup`** - Press and release keys individually
2. **`speed`** - Sets CPU speed to "2.4", "friendly", or "max"
3. **`dis`**, **`x`** - Disassemble code at specified address or current PC
4. **`flags`** - Get/set CPU flags
5. **`lcd_ignore (li)`** - Specify regions on LCD to ignore during monitoring
6. **`lcd_mon (lm)`** - Turn LCD monitoring on/off
7. **`list_break (lb)`** - List all active breakpoints (alias for breakpoints)
8. **`help`** - Show help information

---

## PC-8201A Specific Findings

### System Configuration

- **Model Command**: `model pc8201` sets the emulated model
- **Display**: 40x8 LCD screen
- **Memory**: 64KB address space

### BASIC Interpreter Access

The PC-8201A has BASIC in ROM. Based on the SuperROM documentation:
- The system uses the `SUPNEC.bin` ROM file
- ROM loading is done via `optrom` command
- BASIC is automatically loaded when the ROM is active

### Menu System

The PC-8201A has a menu system with:
- **Arrow keys** for navigation
- **ENTER** to launch programs/files
- **Files** launch associated programs
- **Programs** launch directly

### Keyboard Input

Based on the VirtualT socket interface:
- No direct keyboard input command documented
- Likely need to write to memory locations that represent keyboard buffer
- Or use CPU execution to process keyboard input

---

## Investigation Steps

### Step 1: Basic Connection Test
- Launch VirtualT with `-p` port option
- Connect socket and send basic commands
- Verify "Ok" responses

### Step 2: Model Configuration
- Set model to pc8201
- Verify status shows correct model

### Step 3: ROM Loading
- Load SUPNEC.bin via `optrom` command
- Verify the ROM is loaded

### Step 4: CPU Control
- Halt CPU
- Read registers to verify CPU state

### Step 5: Menu Navigation (Future Work)
- Navigate to BASIC interpreter
- Launch BASIC
- Type and execute program

---

## Initial Test Results

### Test 1: Basic Process Launch
- **Status**: ✅ Success
- Process launches successfully with auto-assigned port
- Socket connection established

### Test 2: Model Setting
- **Status**: ✅ Success
- `model pc8201` command accepted with "Ok" response

### Test 3: Status Query
- **Status**: ✅ Success
- Returns: `Model=pc8201, CPU halted`

### Test 4: ROM Loading
- **Status**: ✅ Success
- `optrom C:/path/to/SUPNEC.bin` accepted with "Ok" response

---

## Known Limitations

1. **Keyboard Input**: The socket interface provides the `key` command for keyboard input, which simulates keystrokes with proper timing for the keyboard ISR. This is the primary method for keyboard input.

2. **No LCD Read Command**: Can write to LCD but cannot read current display state. Need to:
   - Monitor LCD memory directly
   - Track display state in client code

3. **No File System Access**: VirtualT doesn't provide file system commands. Need to:
   - Load programs via ROM
   - Use memory operations to interact with programs

4. **No Direct Memory Read for Keyboard**: Keyboard input is done through the `key` command, not direct memory writes.

---

## Next Steps

1. **Test the `key` command**:
   - Navigate menu with arrow keys
   - Select and launch BASIC
   - Type and execute a program

2. **Test Menu Navigation**:
   - Use `key` command to send arrow keys
   - Navigate to BASIC in the menu
   - Verify BASIC starts

3. **Test BASIC Program Entry**:
   - Type BASIC program line by line using `key`
   - Execute program with `key enter`
   - Verify output on LCD

4. **Create vt_interact Library**:
   - Higher-level menu navigation functions using `key` command
   - BASIC program entry helpers
   - State tracking utilities

5. **Create Regression Test Module**:
   - Test all socket commands
   - Test menu navigation workflow
   - Test BASIC program execution

---

## Conclusion

The VirtualT socket interface provides comprehensive CPU control, register access, memory operations, and basic system control. The PC-8201A can be successfully configured and controlled via the socket interface.

### Key Discovery

The most important finding is the `key` command, which enables full keyboard input for the PC-8201A:

- Simulates keystrokes with proper timing for the keyboard ISR
- Supports quoted strings, special keys, and key sequences
- Enables menu navigation, program launching, and BASIC entry

The socket interface now has all the capabilities needed to fully operate the PC-8201A, including:
- CPU control (halt, run, step, status)
- Register and memory access
- LCD display control
- ROM loading
- Keyboard input via `key` command

The main work remaining is integrating these commands into higher-level operations through the vt_interact library.
