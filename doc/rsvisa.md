# Tcl Interface to Rohde & Schwarz VISA Library

A Tcl interface to the VISA (Virtual Instrument Software Architecture) library for controlling test and measurement instruments.

**Version:** 1.0.0 \
**Package:** `rsvisa` \
**Namespace:** `rsvisa::`

---

## Table of Contents

1. [Overview](#overview)
2. [Key Features](#key-features)
3. [Supported Instruments](#supported-instruments)
4. [Installation](#installation)
5. [Command Reference](#command-reference)
   - 5.1 [Resource Management](#resource-management)
   - 5.2 [Instrument Queries](#instrument-queries)
   - 5.3 [Communication Commands](#communication-commands)
   - 5.4 [Attribute Management](#attribute-management)
6. [Usage Examples](#usage-examples)
7. [Error Handling](#error-handling)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)
10. [License](#license)

---

## Overview

The **rsvisa** package provides a Tcl interface to the VISA (Virtual Instrument Software Architecture) library, specifically targeting the Rohde & Schwarz VISA implementation. VISA is an industry-standard API for controlling test and measurement instruments.

## Key Features

- Hardware-independent instrument control
- Support for multiple communication protocols:
  - GPIB (IEEE 488)
  - USB
  - Ethernet (VXI-11, LXI, SOCKET)
  - RS-232 (Serial)
  - PXI
- Binary data transfer for large datasets (waveforms, spectra, screenshots)
- SCPI (Standard Commands for Programmable Instruments) support
- Automatic buffer management for large data transfers
- Cross-platform support (Windows/Linux)

## Supported Instruments

- Oscilloscopes
- Spectrum analyzers
- Signal generators
- Digital multimeters
- Network analyzers
- Any VISA-compatible laboratory instrument

---

## Installation

```tcl
package require rsvisa
```

### Requirements

**Build-time**:
- Tcl 8.6 or later
- C++11 compatible compiler
- R&S VISA development headers
- Autoconf and GNU Make

**Runtime**:
- Tcl 8.6+
- R&S VISA library:
  - Windows: `RsVisa32.dll`
  - Linux: `librsvisa.so`

### Building from Source

```bash
cd D:\CM.tcltk\tcltk86\external\tclrsvisa
autoconf
./configure --with-rsvisa=/path/to/rsvisa/installation
make
make test
make install
```

---

## Command Reference

### Resource Management

#### `rsvisa::GetResourceManager`

Opens a connection to the VISA Resource Manager.

**Syntax:**
```tcl
set rm [rsvisa::GetResourceManager]
```

**Returns:**

- Resource manager handle (long integer)

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
set rm [rsvisa::GetResourceManager]
```

---

#### `rsvisa::OpenResource`

Opens a connection to a specific instrument and clears I/O buffers.

**Syntax:**
```tcl
set instr [rsvisa::OpenResource resourceManager resourceUri]
```

**Parameters:**

- `resourceManager` - Handle from GetResourceManager
- `resourceUri` - VISA resource string (see Resource URI Formats below)

**Returns:**

- Instrument session handle (long integer)

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]
```

---

#### `rsvisa::CloseResource`

Closes an instrument session and releases resources.

**Syntax:**
```tcl
rsvisa::CloseResource instrumentHandle
```

**Parameters:**

- `instrumentHandle` - Session handle from OpenResource

**Returns:**

- Empty string on success

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
rsvisa::CloseResource $scope
```

---

### Instrument Queries

#### `rsvisa::Idn`

Sends the `*IDN?` command and returns the instrument identification string.

**Syntax:**
```tcl
set id [rsvisa::Idn instrumentHandle]
```

**Parameters:**

- `instrumentHandle` - Session handle

**Returns:**

- Byte array containing instrument ID string
- Format: "Manufacturer,Model,SerialNumber,FirmwareVersion"

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
set id [rsvisa::Idn $scope]
puts "Connected to: $id"
# Output: "Rohde&Schwarz,RTO1024,123456,1.50"
```

---

#### `rsvisa::Stb`

Reads the IEEE 488.2 Status Byte register.

**Syntax:**
```tcl
set status [rsvisa::Stb instrumentHandle]
```

**Parameters:**

- `instrumentHandle` - Session handle

**Returns:**

- Integer (0-255) status byte value

**Status Bits:**
- Bit 6 (0x40): Master Summary Status (MSS)
- Bit 5 (0x20): Event Status Bit (ESB)
- Bit 4 (0x10): Message Available (MAV)

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
set stb [rsvisa::Stb $instr]
if {$stb & 0x10} {
    puts "Data available"
}
```

---

### Communication Commands

#### `rsvisa::WriteRead`

Sends a command and reads the response.

**Syntax:**
```tcl
set response [rsvisa::WriteRead instrumentHandle command]
```

**Parameters:**

- `instrumentHandle` - Session handle
- `command` - SCPI command string

**Returns:**

- Byte array containing instrument response

**Buffer:**
- Initial: 1KB (1024 bytes)
- Auto-expands by 1KB increments if needed

**Errors:**

- Returns VISA error code or "Out of memory"

**Example:**
```tcl
set voltage [rsvisa::WriteRead $scope "MEASure:VOLTage:DC?\n"]
puts "Voltage: $voltage"
```

---

#### `rsvisa::WriteReadBin`

Optimized for large binary data transfers (waveforms, screenshots, spectra).

**Syntax:**
```tcl
set data [rsvisa::WriteReadBin instrumentHandle command]
```

**Parameters:**

- `instrumentHandle` - Session handle
- `command` - SCPI command string

**Returns:**

- Byte array containing binary data

**Buffer:**
- Initial: 1MB (1,000,000 bytes)
- Auto-expands by 1KB increments if needed

**Errors:**

- Returns VISA error code or "Out of memory"

**Example:**
```tcl
set waveform [rsvisa::WriteReadBin $scope "WAVeform:DATA?\n"]
set fp [open "waveform.bin" wb]
puts -nonewline $fp $waveform
close $fp
```

---

#### `rsvisa::Write`

Sends a command without waiting for a response.

**Syntax:**
```tcl
rsvisa::Write instrumentHandle command
```

**Parameters:**

- `instrumentHandle` - Session handle
- `command` - SCPI command string

**Returns:**

- Empty string on success

**Use Case:**
- Commands that don't generate responses
- Configuration commands

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
rsvisa::Write $scope "CHANnel1:RANGe 10\n"
rsvisa::Write $scope "TRIGger:MODE AUTO\n"
```

---

#### `rsvisa::Read`

Reads data without sending a command first.

**Syntax:**
```tcl
set data [rsvisa::Read instrumentHandle]
```

**Parameters:**

- `instrumentHandle` - Session handle

**Returns:**

- Byte array containing instrument data

**Buffer:**
- Initial: 1KB
- Auto-expands by 1KB increments if needed

**Use Case:**
- After Write command
- For asynchronous data

**Errors:**

- Returns VISA error code or "Out of memory"

**Example:**
```tcl
rsvisa::Write $instr "INITiate\n"
# ... wait for measurement ...
set result [rsvisa::Read $instr]
```

---

### Attribute Management

#### `rsvisa::SetAttribute`

Sets a VISA session attribute.

**Syntax:**
```tcl
rsvisa::SetAttribute instrumentHandle attribute value
```

**Parameters:**

- `instrumentHandle` - Session handle
- `attribute` - VISA attribute ID (integer from visa.h)
- `value` - Attribute value (integer)

**Common Attributes:**
- `VI_ATTR_TMO_VALUE` - Timeout value in milliseconds
- `VI_ATTR_TERMCHAR` - Termination character (e.g., 0x0A for LF)
- `VI_ATTR_TERMCHAR_EN` - Enable/disable termination character

**Returns:**

- Empty string on success

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
# Set timeout to 3 seconds
rsvisa::SetAttribute $instr 0x3FFF001A 3000
```

---

#### `rsvisa::GetAttribute`

Retrieves a VISA session attribute value.

**Syntax:**
```tcl
set value [rsvisa::GetAttribute instrumentHandle attribute]
```

**Parameters:**

- `instrumentHandle` - Session handle
- `attribute` - VISA attribute ID (integer)

**Returns:**

- 64-bit integer value of attribute

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
set timeout [rsvisa::GetAttribute $instr 0x3FFF001A]
puts "Timeout: $timeout ms"
```

---

#### `rsvisa::SetTimeout`

Convenience wrapper for setting the timeout attribute.

**Syntax:**
```tcl
rsvisa::SetTimeout instrumentHandle timeout
```

**Parameters:**

- `instrumentHandle` - Session handle
- `timeout` - Timeout in milliseconds (integer)

**Special Values:**
- `VI_TMO_INFINITE` (0xFFFFFFFF) - No timeout
- `VI_TMO_IMMEDIATE` (0) - Return immediately

**Typical Values:**
- 2000-5000 ms for most instruments
- 10000+ ms for large data transfers

**Returns:**

- Empty string on success

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
rsvisa::SetTimeout $scope 5000  ;# 5 seconds
```

---

#### `rsvisa::GetTimeout`

Convenience wrapper for getting the timeout attribute.

**Syntax:**
```tcl
set timeout [rsvisa::GetTimeout instrumentHandle]
```

**Parameters:**

- `instrumentHandle` - Session handle

**Returns:**

- Timeout value in milliseconds (long integer)

**Errors:**

- Returns VISA error code on failure

**Example:**
```tcl
set timeout [rsvisa::GetTimeout $scope]
puts "Current timeout: $timeout ms"
```

---

## Resource URI Formats

VISA resource strings identify instruments and their connection methods:

### Ethernet/LAN

**VXI-11 Protocol:**
```
TCPIP::192.168.1.100::INSTR
TCPIP::hostname.domain.com::INSTR
```

**Socket Protocol:**
```
TCPIP::192.168.1.100::5025::SOCKET
```

### USB

```
USB::0x1234::0x5678::SN123456::INSTR
```
- `0x1234` - Vendor ID
- `0x5678` - Product ID
- `SN123456` - Serial number

### GPIB

```
GPIB::10::INSTR
```
- `10` - GPIB address (0-30)

### Serial Port

**Windows:**
```
ASRL1::INSTR   (COM1)
ASRL2::INSTR   (COM2)
```

**Linux:**
```
ASRL/dev/ttyS0::INSTR
ASRL/dev/ttyUSB0::INSTR
```

### PXI

```
PXI::0::INSTR
PXI::1::BACKPLANE
```

---

## Complete Examples

### Example 1: Basic Connection and Query

```tcl
package require rsvisa

# Open resource manager
set rm [rsvisa::GetResourceManager]

# Connect to oscilloscope via Ethernet
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]

# Query identification
set id [rsvisa::Idn $scope]
puts "Connected to: $id"

# Set timeout to 5 seconds
rsvisa::SetTimeout $scope 5000

# Query measurement
set voltage [rsvisa::WriteRead $scope "MEASure:VOLTage:DC?\n"]
puts "DC Voltage: $voltage V"

# Close connection
rsvisa::CloseResource $scope
```

---

### Example 2: Binary Waveform Capture

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]

# Set longer timeout for large transfer
rsvisa::SetTimeout $scope 10000

# Configure waveform format
rsvisa::Write $scope "WAVeform:FORMat BYTE\n"
rsvisa::Write $scope "WAVeform:SOURce CHANnel1\n"

# Capture waveform data (binary)
set waveform [rsvisa::WriteReadBin $scope "WAVeform:DATA?\n"]

# Save to file
set fp [open "waveform.bin" wb]
puts -nonewline $fp $waveform
close $fp

puts "Captured [string length $waveform] bytes"

rsvisa::CloseResource $scope
```

---

### Example 3: Status Polling

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]
set instr [rsvisa::OpenResource $rm "GPIB::10::INSTR"]

# Set timeout
rsvisa::SetTimeout $instr 30000

# Initiate long measurement
rsvisa::Write $instr "INITiate:IMMediate\n"

# Poll status until complete
puts "Waiting for measurement..."
while {1} {
    set stb [rsvisa::Stb $instr]
    if {$stb & 0x10} {
        # MAV bit set - data available
        puts "Measurement complete!"
        break
    }
    after 100  ;# Wait 100ms
}

# Read result
set result [rsvisa::Read $instr]
puts "Result: $result"

rsvisa::CloseResource $instr
```

---

### Example 4: Multiple Instruments

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]

# Connect to oscilloscope
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]
rsvisa::SetTimeout $scope 5000

# Connect to signal generator
set gen [rsvisa::OpenResource $rm "TCPIP::192.168.1.101::INSTR"]
rsvisa::SetTimeout $gen 5000

# Configure generator
rsvisa::Write $gen "FREQuency 1000000\n"      ;# 1 MHz
rsvisa::Write $gen "VOLTage 0.5\n"             ;# 0.5 Vpp
rsvisa::Write $gen "OUTPut ON\n"

# Measure on oscilloscope
set freq [rsvisa::WriteRead $scope "MEASure:FREQuency?\n"]
set ampl [rsvisa::WriteRead $scope "MEASure:VAMPlitude?\n"]

puts "Measured: $freq Hz, $ampl V"

# Cleanup
rsvisa::Write $gen "OUTPut OFF\n"
rsvisa::CloseResource $scope
rsvisa::CloseResource $gen
```

---

### Example 5: Error Handling

```tcl
package require rsvisa

proc connect_instrument {uri} {
    if {[catch {
        set rm [rsvisa::GetResourceManager]
        set instr [rsvisa::OpenResource $rm $uri]

        # Verify connection
        set idn [rsvisa::Idn $instr]
        puts "Connected to: $idn"

        return $instr
    } error]} {
        puts stderr "Error connecting to $uri: $error"
        return ""
    }
}

# Try to connect
set scope [connect_instrument "TCPIP::192.168.1.100::INSTR"]
if {$scope ne ""} {
    # Use instrument
    set voltage [rsvisa::WriteRead $scope "MEASure:VOLTage?\n"]
    puts "Voltage: $voltage"

    # Cleanup
    rsvisa::CloseResource $scope
} else {
    puts "Failed to connect to instrument"
}
```

---

### Example 6: Screenshot Capture

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]

# Set long timeout for screenshot
rsvisa::SetTimeout $scope 30000

# Request screenshot in PNG format
rsvisa::Write $scope "HCOPy:DEVice:LANGuage PNG\n"
set screenshot [rsvisa::WriteReadBin $scope "HCOPy:DATA?\n"]

# Save to file
set fp [open "screenshot.png" wb]
puts -nonewline $fp $screenshot
close $fp

puts "Screenshot saved ([string length $screenshot] bytes)"

rsvisa::CloseResource $scope
```

---

## VISA Status Codes

### Success Codes
- `VI_SUCCESS` (0) - Operation completed successfully
- `VI_SUCCESS_MAX_CNT` - Maximum count reached (partial success)

### Common Error Codes
- `VI_ERROR_RSRC_NFOUND` (-1073807343) - Resource not found
- `VI_ERROR_TMO` (-1073807339) - Timeout occurred
- `VI_ERROR_CONN_LOST` (-1073807194) - Connection lost
- `VI_ERROR_INV_OBJECT` (-1073807346) - Invalid session/object

---

## Best Practices

### 1. Always Close Resources
```tcl
# Good practice
set instr [rsvisa::OpenResource $rm $uri]
# ... use instrument ...
rsvisa::CloseResource $instr

# Better practice with error handling
if {[catch {
    set instr [rsvisa::OpenResource $rm $uri]
    # ... use instrument ...
} error]} {
    puts stderr "Error: $error"
}
# Cleanup always happens
catch {rsvisa::CloseResource $instr}
```

### 2. Set Appropriate Timeouts
```tcl
# Fast queries - short timeout
rsvisa::SetTimeout $instr 2000

# Large data transfers - long timeout
rsvisa::SetTimeout $instr 30000

# Very long measurements - very long or infinite
rsvisa::SetTimeout $instr 0xFFFFFFFF  ;# VI_TMO_INFINITE
```

### 3. Use WriteReadBin for Binary Data
```tcl
# Good - uses optimized binary transfer
set waveform [rsvisa::WriteReadBin $scope "WAVeform:DATA?\n"]

# Less efficient - smaller initial buffer
set waveform [rsvisa::WriteRead $scope "WAVeform:DATA?\n"]
```

### 4. Terminate Commands Properly
```tcl
# Include newline at end of commands
rsvisa::Write $instr "FREQuency 1000\n"

# Not all instruments require newline - check manual
rsvisa::Write $instr "FREQ 1000"
```

### 5. Check Status Before Reading
```tcl
# Poll status to avoid unnecessary timeouts
while {1} {
    set stb [rsvisa::Stb $instr]
    if {$stb & 0x10} break  ;# MAV bit
    after 100
}
set result [rsvisa::Read $instr]
```

---

## Troubleshooting

### Connection Failed

**Problem:** `VI_ERROR_RSRC_NFOUND` when opening resource

**Solutions:**
- Verify instrument is powered on
- Check network connectivity (ping IP address)
- Verify VISA resource string format
- Check firewall settings (VXI-11 uses port 111)
- Ensure R&S VISA library is installed

### Timeout Errors

**Problem:** `VI_ERROR_TMO` during read operations

**Solutions:**
- Increase timeout: `rsvisa::SetTimeout $instr 10000`
- Verify instrument is responding (check front panel)
- Check if command requires `*WAI` or `*OPC?`
- For long operations, use status polling instead

### Binary Data Corruption

**Problem:** Binary data appears corrupted

**Solutions:**
- Use `WriteReadBin` instead of `WriteRead`
- Save binary data with `puts -nonewline` and binary mode: `open "file" wb`
- Check if instrument sends IEEE block format header

### Permission Denied

**Problem:** Cannot access VISA resources

**Solutions:**
- Windows: Run as administrator or check VISA permissions
- Linux: Add user to appropriate group (dialout, usb, etc.)
- Check antivirus/security software blocking access

---

## Technical Notes

### Binary Data Handling
All read commands return Tcl byte arrays, preserving binary data integrity. Binary data must be written to files in binary mode.

### Automatic Buffer Management
Read operations automatically expand buffers as needed. Initial sizes:
- `WriteRead`: 1KB
- `WriteReadBin`: 1MB
- `Read`: 1KB
- `Idn`: 1KB (fixed)

### Thread Safety
The package is built with thread support when Tcl is thread-enabled.

### R&S VISA vs. IVI VISA
This package is designed specifically for Rohde & Schwarz VISA and uses different library names (`RsVisa32.dll` vs. `visa32.dll`).

---

## See Also

- [SCPI Standard](https://www.ivifoundation.org/scpi/)
- [VISA Specification](https://www.ivifoundation.org/specifications/)
- [Rohde & Schwarz VISA](https://www.rohde-schwarz.com/drivers/visa/)

---

## License

Copyright (c) 2002 IVI Foundation

Licensed under the IVI Foundation License (permissive open source license).

---

## Version History

**1.0.0** (2025)
- Initial release
- Support for all VISA communication protocols
- Binary data transfer support
- Comprehensive error handling
