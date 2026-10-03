# Virtual T Socket Interface

This document describes the socket interface supported by VirtualT for remote control of the emulator.

---

## Overview

The Socket Interface provides the capability of remote control of most of VirtualT's emulation functionality. The interface also adds low-level debug control of the emulation.

---

## Invoking

To enable the socket interface, VirtualT must be invoked with a command-line option to specify the socket port:

```
virtualt -p port_number
```

**Note:** On Windows platforms, the Windows firewall sometimes blocks unknown ports and may need to be configured. Sometimes simply invoking VirtualT with the `-p` option a second time will "open" the port.

This will cause VirtualT to launch a socket listener thread for remote control applications.

To connect to the socket interface using the vt_client application:

```
cd VirtualT_directory
./vt_client port_number
```

The vt_client application connects to the socket interface of VirtualT and presents a command-line interface for control. The socket interface can be used by any client (Perl script, C++ test bench, or any socket-capable program).

---

## Protocol

The socket interface protocol is an ASCII-based command/response type protocol.

**Command Rules:**
- Commands must be sent as independent socket messages
- Commands are not terminated with a NEWLINE character
- Each command can have one or more arguments

**Response Rules:**
- All responses end with the ASCII text "Ok"
- Responses that report information have one or more lines of data, each followed by a NEWLINE (`\n`) character

### Example Transaction

```
Client   sends "halt"
VirtualT sends "Ok"
Client   sends "pc"
VirtualT sends "27105\n"
VirtualT sends "Ok"
Client   sends "radix 16"
VirtualT sends "Ok"
Client   sends "wr pc=0x69e2"
VirtualT sends "Ok"
Client   sends "run"
VirtualT sends "Ok"
```

---

## Async Events

Occasionally VirtualT needs to send a message to the client to indicate an event has occurred. The format for async event reporting is:

```
event, event_name, event_data \n
```

The async events reported by VirtualT 1.0 are:
- `event, break, PC=address` - Indicates a breakpoint was encountered
- `event, lcdwrite, (r,c),data` - Reports data written to the LCD at (r,c)

## Commands

### Emulation Control Commands

#### `halt (h)`
- **Parameters:** none
- **Returns:** Ok
- Halts execution of the 8085 CPU. If the CPU is already halted, this command has no effect.

#### `run (r)`
- **Parameters:** none
- **Returns:** Ok
- Resumes the CPU execution after a halt command or a breakpoint. If the CPU is already running, this operation has no affect.

#### `step (s)`
- **Parameters:** [count]
- **Returns:** Parameter Error, Ok
- Causes the CPU to execute one or more instructions beginning at the current Program Counter (PC) location. If no parameter is provided, a single instruction is executed.

#### `step_over (so)`
- **Parameters:** [count]
- **Returns:** Parameter Error, Ok
- Causes the CPU to step over any RST, CALL, CZ, CNZ, etc. instruction and return control at the next instruction. The subroutine which is the target of the RST, CALL, etc. will be executed entirely.

#### `status`
- **Parameters:** none
- **Returns:** Current model and CPU status, Ok
- Returns the current model and CPU running status in the following format:
  ```
  Model=m100, CPU running
  Model=pc8201, CPU halted
  ```

#### `reset`
- **Parameters:** none
- **Returns:** Ok
- Performs an emulation system reset. Does not clear emulation memory.

#### `cold_boot`
- **Parameters:** none
- **Returns:** Ok
- This command is used to cold boot the current emulation.

#### `terminate`
- **Parameters:** none
- **Returns:** Ok
- Terminates the VirtualT session and forces a clean shutdown (RAM and Preferences will be saved).

#### `model`
- **Parameters:** m100, pc8201, m200
- **Returns:** Ok
- Sets the emulated model.

#### `cpu`
- **Parameters:** friendly, fast, normal
- **Returns:** Ok
- Sets the emulation speed to the specified value. Note that only one option is given for the CPU Friendly speed but the Speed menu item provides two selections. The friendly option in this command will set the speed to match the menu item "CPU Friendly".

#### `radix`
- **Parameters:** "10", "16"
- **Returns:** Parameter error, Ok
- Specifies the radix that should be used when reporting addresses and data to the socket interface. The radix parameter does not affect input data. Input data will be parsed using standard C regardless of the radix setting. The default radix is 10 (decimal).

#### `load`
- **Parameters:** filename
- **Returns:** Load Error, Ok
- Loads the specified file from the Host OS into the emulated model's "filesystem". The file is loaded relative to the VirtualT working directory. As with any "Load from Host" operation, this will force a system reset after the load is complete.

#### `optrom`
- **Parameters:** ["unload", filename]
- **Returns:** none, Current option ROM filename, Invalid hex file, File not found, Ok
- If the command is sent with no parameters, VirtualT returns the name of the currently loaded option ROM or "none" if no option ROM is loaded. If the parameter "unload" is sent, VirtualT will unload the option ROM. If a filename is provided, that file will be loaded into the Option ROM memory space, or an error will be returned if it cannot be loaded. Unlike the menu item "Load / Unload option ROM", this command does not reset the CPU after loading or unloading a ROM.

#### `in`
- **Parameters:** portNumber
- **Returns:** Parameter error, Port read value, Ok
- Reads the value of the specified port and returns the value using the current selected radix.

#### `out`
- **Parameters:** address, value
- **Returns:** Parameter error, Ok
- Performs a CPU 'out' instruction to the specified port, writing the specified data. The out instruction is performed asynchronously from the emulation.

#### `key`
- **Parameters:** Keystroke list
- **Returns:** Parameter error, Ok
- Simulates keystroke events on the keyboard. The keystroke list can be any length and can consist of quoted strings and/or special key symbols as described below.

**Keystroke Format:**
- Quoted strings: `"test"` - types the text
- Special keys: `enter`, `shift`, `code`, `graph`, `grph`, `esc`, `ctrl`, `tab`, `f1`-`f8`, `paste`, `label`, `print`, `pause`, `left`, `right`, `up`, `down`, `space`, `insert`, `ins`, `delete`, `del`, `bksp`, `back`, `backspace`, `home`, `end`, `pgup`, `pageup`, `pgdn`, `pagedown`
- Chain keys with `+`: `ctrl+c`, `graph+a`
- Chain sequences: `right enter "test.do" f8`
- To specify the quote character as input in a quoted string, precede the quote with the `\` character. To specify the `\` character type `\\`.

**Examples:**
```
key ctrl+c
key graph+a
key right enter "test.do" enter "This is data for \"TEXT\"" f8
```

---

### Register Operations

#### `halt (h)`
- **Parameters:** none
- **Returns:** Ok
- Halts the CPU

#### `run (r)`
- **Parameters:** none
- **Returns:** Ok
- Runs the CPU

#### `step (s)`
- **Parameters:** [count]
- **Returns:** Parameter Error, Ok
- Executes one or more instructions. If no count is provided, a single instruction is executed.

#### `step_over (so)`
- **Parameters:** [count]
- **Returns:** Parameter Error, Ok
- Steps over any RST, CALL, CZ, CNZ, etc. instruction and returns control at the next instruction.

#### `status`
- **Parameters:** none
- **Returns:** Current model and CPU status, Ok
- Returns the current model and CPU running status in the format:
  ```
  Model=m100, CPU running
  Model=pc8201, CPU halted
  ```

#### `terminate`
- **Parameters:** none
- **Returns:** Ok
- Terminates the VirtualT session and forces a clean shutdown (RAM and Preferences will be saved).

#### `reset`
- **Parameters:** none
- **Returns:** Ok
- Performs an emulation system reset. Does not clear emulation memory.

---

### Register Operations

#### `a`, `b`, `c`, `d`, `e`, `h`, `l`, `m`
- **Parameters:** none
- **Returns:** Register value in decimal, Ok
- Returns the value of the specified register

#### `bc`, `de`, `hl`, `sp`, `pc`
- **Parameters:** none
- **Returns:** Register value in decimal, Ok
- Returns the value of the specified register pair

#### `radix`
- **Parameters:** "8", "10", "16"
- **Returns:** Ok
- Sets the display radix for register values

#### `write_reg (wr)`
- **Parameters:** a=xx b=xx c=xx d=xx e=xx h=xx l=xx m=xx or bc=xx de=xx hl=xx sp=xx pc=xx
- **Returns:** Ok
- Writes data to one or more CPU registers. Register write parameters must be separated by spaces and must follow the specified syntax. Input values will be parsed using standard C notation.

#### `write_mem (wm)`
- **Parameters:** address data [data data ...]
- **Returns:** Ok
- Writes data to the CPU's 64K memory space starting with the specified address. For Base Memory emulation, writes to the System ROM are allowed using this command. For ReMem memory emulation, write operations will be dictated by the READ_ONLY bit for the address block being written.
No provisions are made by this command to halt the CPU while writing multiple memory locations.

---

### Memory Operations

---

### Memory Operations

#### `read_mem (rm)`
- **Parameters:** address count
- **Returns:** Memory data, Ok
- Reads memory and returns data in hex format

#### `string`
- **Parameters:** address
- **Returns:** String data, Ok
- Reads data from memory and formats it as a NULL-terminated string

#### `dasm`
- **Parameters:** address [count]
- **Returns:** Disassembly, Ok
- Disassembles code starting at the specified address

---

### Breakpoint Operations

#### `break`
- **Parameters:** address
- **Returns:** Ok
- Sets a breakpoint at the specified address

#### `clearbreak`
- **Parameters:** address
- **Returns:** Ok
- Clears a breakpoint at the specified address

#### `clearallbreak`
- **Parameters:** none
- **Returns:** Ok
- Clears all breakpoints

#### `breakpoints`
- **Parameters:** none
- **Returns:** Breakpoint list, Ok
- Lists all active breakpoints

---

### System Control

#### `cpu [friendly|fast|normal]`
- **Parameters:** friendly, fast, or normal
- **Returns:** Ok
- Sets the CPU speed

#### `model [m100|pc8201|m200]`
- **Parameters:** m100, pc8201, or m200
- **Returns:** Ok
- Sets the emulated model

#### `cpu [friendly|fast|normal]`
- **Parameters:** friendly, fast, or normal
- **Returns:** Ok
- Sets the CPU speed

#### `radix`
- **Parameters:** "10", "16"
- **Returns:** Parameter error, Ok
- Specifies the radix that should be used when reporting addresses and data to the
  socket interface. The default radix is 10 (decimal).

#### `reset`
- **Parameters:** none
- **Returns:** Ok
- Performs an emulation system reset. Does not clear emulation memory.

#### `cold_boot`
- **Parameters:** none
- **Returns:** Ok
- Performs a cold boot of the current emulation.

#### `terminate`
- **Parameters:** none
- **Returns:** Ok
- Terminates the VirtualT session and forces a clean shutdown (RAM and Preferences will be saved).

#### `in`
- **Parameters:** portNumber
- **Returns:** Parameter error, Port read value, Ok
- Reads the value of the specified port and returns the value using the current selected radix.

#### `out`
- **Parameters:** address, value
- **Returns:** Parameter error, Ok
- Performs a CPU 'out' instruction to the specified port, writing the specified data.
  The out instruction is performed asynchronously from the emulation.

---

### File Operations

#### `lcd`
- **Parameters:** row col data
- **Returns:** Ok
- Writes data to the LCD at the specified position

#### `lcd_clear`
- **Parameters:** none
- **Returns:** Ok
- Clears the LCD display

---

### Key Input (PC-8201A Keyboard)

#### `load_rom`
- **Parameters:** filename
- **Returns:** Ok
- Loads a ROM file

#### `save_ram`
- **Parameters:** filename
- **Returns:** Ok
- Saves RAM to a file

#### `load_ram`
- **Parameters:** filename
- **Returns:** Ok
- Loads RAM from a file

---

### Information Commands

#### `pc`
- **Parameters:** none
- **Returns:** PC value in decimal, Ok
- Returns the current program counter

#### `regs`
- **Parameters:** none
- **Returns:** All register values, Ok
- Returns all CPU register values

#### `status`
- **Parameters:** none
- **Returns:** Model and CPU status, Ok
- Returns current emulation status

---

### Key Input (PC-8201A Keyboard)

#### `key`
- **Parameters:** Keystroke list
- **Returns:** Parameter error, Ok
- Simulates keystroke events on the keyboard. The keystroke list can be any length
  and can consist of quoted strings and/or special key symbols.

**Keystroke Format:**
- Quoted strings: `"test"` - types the text
- Special keys: `enter`, `shift`, `graph`, `esc`, `ctrl`, `tab`, `f1`-`f8`, `left`, `right`, `up`, `down`, `space`, `insert`, `delete`, `backspace`, `home`, `end`, `pgup`, `pgdn`
- Chain keys with `+`: `ctrl+c`, `graph+a`
- Chain sequences: `right enter "test.do" f8`

**Examples:**
```
key ctrl+c
key graph+a
key right enter "test.do" enter "This is data for \"TEXT\"" f8
```

#### `keydown`
- **Parameters:** Key name
- **Returns:** Ok
- Presses and holds a key

#### `keyup`
- **Parameters:** Key name
- **Returns:** Ok
- Releases a held key

---

### LCD Operations

- No provisions are made by memory write commands to halt the CPU while writing multiple locations
- Input values are parsed using standard C notation (hex with 0x prefix, octal with 0 prefix)
- The socket interface works with the integrated debugger for remote debug control

---

*Document generated from VirtualT 1.3 documentation*