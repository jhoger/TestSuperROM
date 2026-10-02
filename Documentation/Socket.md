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

## Commands

### CPU Control

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

#### `radix [8|10|16]`
- **Parameters:** 8, 10, or 16
- **Returns:** Ok
- Sets the display radix for register values

#### `write_reg (wr)`
- **Parameters:** a=xx b=xx c=xx d=xx e=xx h=xx l=xx m=xx or bc=xx de=xx hl=xx sp=xx pc=xx
- **Returns:** Ok
- Writes data to one or more CPU registers

#### `write_mem (wm)`
- **Parameters:** address data [data data ...]
- **Returns:** Ok
- Writes data to the CPU's 64K memory space starting at the specified address

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

---

### LCD Operations

#### `lcd`
- **Parameters:** row col data
- **Returns:** Ok
- Writes data to the LCD at the specified position

#### `lcd_clear`
- **Parameters:** none
- **Returns:** Ok
- Clears the LCD display

---

### File Operations

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

## Notes

- No provisions are made by memory write commands to halt the CPU while writing multiple locations
- Input values are parsed using standard C notation (hex with 0x prefix, octal with 0 prefix)
- The socket interface works with the integrated debugger for remote debug control

---

*Document generated from VirtualT 1.3 documentation*