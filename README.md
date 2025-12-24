# tclrsvisa - Tcl Extension for VISA Instrument Control

A Tcl extension that provides a programmatic interface to the VISA (Virtual Instrument Software Architecture) library, specifically targeting the Rohde & Schwarz VISA implementation. This extension enables Tcl scripts to communicate with and control test and measurement instruments such as oscilloscopes, spectrum analyzers, signal generators, and other VISA-compatible devices.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [About VISA](#about-visa)
- [R&S VISA vs. Standard IVI VISA](#rs-visa-vs-standard-ivi-visa)
- [Building from Source](#building-from-source)
- [Installation](#installation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Examples](#examples)
- [License](#license)

## Features

- **13 Core Commands** for comprehensive instrument control:
  - Resource management (open/close connections)
  - Instrument queries (ID, status)
  - Read/Write operations (ASCII and binary)
  - Attribute management (timeout, termination, etc.)
- **Cross-platform Support**: Windows (MSYS2/MinGW) and Linux/Unix
- **Multiple Transport Protocols**: GPIB, USB, Ethernet (VXI-11, LXI), RS-232, PXI
- **Binary Data Support**: Optimized for large data transfers (waveforms, spectra)
- **Hardware-independent**: Works with any VISA-compatible instrument
- **Tcl Stubs**: Dynamic linking for better compatibility

## Requirements

### Software Dependencies

- **Tcl 8.6 or later** - [Download Tcl](https://www.tcl-lang.org/software/tcltk/)
- **R&S VISA Library** - See download links below
- **Build tools** (for compilation):
  - MinGW-w64 (Windows) or GCC (Linux/Unix)
  - GNU Autoconf and Make
  - C++11 compatible compiler

### R&S VISA Downloads

Download the Rohde & Schwarz VISA library from:

- **Main Download Page**: [R&S VISA and Tools](https://www.rohde-schwarz.com/de/driver-pages/fernsteuerung/3-visa-und-tools_231388.html)
- **Application Note**: [R&S VISA Application Note](https://www.rohde-schwarz.com/fi/applications/r-s-visa-application-note_56280-148812.html)

The R&S VISA package includes:
- VISA library (Windows: `RsVisa32.dll`, Linux: `librsvisa.so`)
- VISA header files
- Documentation and tools
- Support for all major instrument communication protocols

## About VISA

VISA (Virtual Instrument Software Architecture) is an industry-standard API for instrument control, maintained by the [IVI Foundation](https://www.ivifoundation.org/). VISA provides:

- **Hardware Abstraction**: Single API for multiple physical transports
- **Industry Standard**: Supported by all major test equipment manufacturers
- **Protocol Independence**: Write once, use with any supported transport
- **Session Management**: Efficient resource and connection handling

For more information, visit the [IVI Foundation website](https://www.ivifoundation.org/).

## R&S VISA vs. Standard IVI VISA

This extension specifically targets the **Rohde & Schwarz VISA** implementation, which differs from the standard IVI VISA in several important ways:

### Header File Differences

**Standard IVI VISA headers** (typically located at `D:\Programs\msys2\mingw64\include\visa`):
```
visa.h          # Standard IVI VISA API
visatype.h      # Standard type definitions
```

**R&S VISA headers** (typically located at `D:\Programs\msys2\mingw64\include\rsvisa`):
```
visa.h          # R&S-specific VISA API
visatype.h      # R&S-specific type definitions
RsVisa.h        # Additional R&S extensions
```

### Key Differences

1. **API Extensions**: R&S VISA includes additional functionality beyond the standard IVI specification
2. **Performance Optimizations**: R&S implementation may have specific optimizations for R&S instruments
3. **Binary Compatibility**: The two implementations are generally compatible but may have subtle differences
4. **Library Names**:
   - Standard IVI: Often `visa32.dll` (Windows) or `libvisa.so` (Linux)
   - R&S VISA: `RsVisa32.dll` (Windows) or `librsvisa.so` (Linux)

**Important**: When compiling this extension, ensure you link against the R&S VISA library and use the R&S header files. The `configure.ac` script is set up to automatically use:
- Windows: `-lRsVisa32`
- Linux: `-lrsvisa`

## Building from Source

### Prerequisites

This project uses the **TEA (Tcl Extension Architecture)** build system and requires a **MinGW-w64** environment on Windows.

#### MinGW-w64 Build Environment (Windows)

The recommended build environment is **MSYS2** with MinGW-w64:

1. **Install MSYS2**: Download from [msys2.org](https://www.msys2.org/)

2. **Install build tools** in MSYS2 MINGW64 shell:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-tcl
   pacman -S autoconf
   pacman -S make
   ```

3. **Install R&S VISA library** (see download links above)

4. **Ensure R&S VISA headers are available**:
   ```bash
   # Headers should be in:
   # D:\Programs\msys2\mingw64\include\rsvisa\
   # or a path that your compiler can find
   ```

### Build Steps

1. **Generate the configure script** (if not present):
   ```bash
   autoconf
   ```

2. **Configure the build**:
   ```bash
   ./configure --prefix=/mingw64
   ```

   Optional: Specify custom R&S VISA path if needed:
   ```bash
   ./configure --prefix=/mingw64 --with-rsvisa=/path/to/rsvisa
   ```

3. **Compile**:
   ```bash
   make
   ```

4. **Test** (requires VISA hardware or simulator):
   ```bash
   make test
   ```

5. **Install**:
   ```bash
   make install
   ```

### Build Configuration

The build system automatically:
- Links to Tcl stubs library (`libtclstub86.a`)
- Uses C++11 standard
- Selects appropriate VISA library based on platform:
  - Windows: `RsVisa32.lib`
  - Linux: `librsvisa`
- Generates shared library (`.dll` on Windows, `.so` on Linux)

## Installation

After building, install the extension:

```bash
make install
```

This will install:
- The compiled library to `$prefix/lib/rsvisa1.0/`
- Documentation to `$prefix/share/man/mann/`
- Package index file for Tcl package autoloading

## Usage

### Basic Usage

```tcl
# Load the extension
package require rsvisa

# 1. Open Resource Manager
set rm [rsvisa::GetResourceManager]

# 2. Connect to instrument (example: LAN-based oscilloscope)
set instr [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]

# 3. Query instrument ID
set id [rsvisa::Idn $instr]
puts "Connected to: $id"

# 4. Send commands and read responses
set result [rsvisa::WriteRead $instr "MEASure:VOLTage:DC?\n"]
puts "Measured voltage: $result"

# 5. Close connection
rsvisa::CloseResource $instr
```

### Setting Communication Timeout

```tcl
# Set timeout to 5 seconds (5000 milliseconds)
rsvisa::SetTimeout $instr 5000

# Read current timeout
set timeout [rsvisa::GetTimeout $instr]
puts "Current timeout: $timeout ms"
```

### Binary Data Transfer

For large data transfers (e.g., waveforms):

```tcl
# Request waveform data (binary format)
set waveform [rsvisa::WriteReadBin $instr "WAVeform:DATA?\n"]

# Process binary data
set length [string length $waveform]
puts "Received $length bytes of waveform data"
```

### Error Handling

```tcl
if {[catch {
    set instr [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]
} error]} {
    puts "Error opening instrument: $error"
    # Error message includes VISA error code
}
```

## API Reference

### Resource Management

| Command | Description | Returns |
|---------|-------------|---------|
| `rsvisa::GetResourceManager` | Opens VISA Resource Manager | Resource manager handle |
| `rsvisa::OpenResource rm uri` | Opens connection to instrument | Instrument handle |
| `rsvisa::CloseResource handle` | Closes instrument connection | None |

### Instrument Queries

| Command | Description | Returns |
|---------|-------------|---------|
| `rsvisa::Idn handle` | Queries instrument ID (*IDN?) | ID string |
| `rsvisa::Stb handle` | Reads Status Byte register | Status byte value |

### Communication

| Command | Description | Returns |
|---------|-------------|---------|
| `rsvisa::Write handle cmd` | Sends command without waiting for response | None |
| `rsvisa::Read handle` | Reads data from instrument | Response string |
| `rsvisa::WriteRead handle cmd` | Sends command and reads response | Response string |
| `rsvisa::WriteReadBin handle cmd` | Like WriteRead but optimized for binary data (1MB buffer) | Binary data |

### Attribute Management

| Command | Description | Returns |
|---------|-------------|---------|
| `rsvisa::SetAttribute handle attr value` | Sets VISA session attribute | None |
| `rsvisa::GetAttribute handle attr` | Gets VISA session attribute | Attribute value |
| `rsvisa::SetTimeout handle ms` | Sets communication timeout | None |
| `rsvisa::GetTimeout handle` | Gets communication timeout | Timeout in ms |

### Resource URI Formats

- **LAN/Ethernet**: `TCPIP::192.168.1.100::INSTR` or `TCPIP::hostname::5025::SOCKET`
- **USB**: `USB::0x1234::0x5678::SN123456::INSTR`
- **GPIB**: `GPIB::10::INSTR` (address 10)
- **Serial**: `ASRL1::INSTR` (COM1/ttyS0)

## Examples

### Example 1: Simple Query

```tcl
package require rsvisa

# Connect to spectrum analyzer via LAN
set rm [rsvisa::GetResourceManager]
set sa [rsvisa::OpenResource $rm "TCPIP::192.168.1.50::INSTR"]

# Query center frequency
set freq [rsvisa::WriteRead $sa "FREQ:CENT?\n"]
puts "Center frequency: $freq Hz"

rsvisa::CloseResource $sa
```

### Example 2: Capture Oscilloscope Waveform

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]

# Set timeout for large data transfer
rsvisa::SetTimeout $scope 10000

# Configure waveform transfer
rsvisa::Write $scope "WAVeform:FORMat BYTE\n"
rsvisa::Write $scope "WAVeform:SOURce CHANnel1\n"

# Capture waveform
set waveform [rsvisa::WriteReadBin $scope "WAVeform:DATA?\n"]

# Save to file
set fp [open "waveform.bin" wb]
puts -nonewline $fp $waveform
close $fp

puts "Captured [string length $waveform] bytes"

rsvisa::CloseResource $scope
```

### Example 3: Status Checking

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]
set instr [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]

# Initiate operation
rsvisa::Write $instr "INITiate:IMMediate\n"

# Poll status until complete
while {1} {
    set stb [rsvisa::Stb $instr]
    if {$stb & 0x10} {
        # Operation complete bit set
        break
    }
    after 100
}

# Read result
set result [rsvisa::WriteRead $instr "FETCh?\n"]
puts "Result: $result"

rsvisa::CloseResource $instr
```

### Example 4: Multiple Instruments

```tcl
package require rsvisa

set rm [rsvisa::GetResourceManager]

# Open multiple instruments
set scope [rsvisa::OpenResource $rm "TCPIP::192.168.1.100::INSTR"]
set generator [rsvisa::OpenResource $rm "TCPIP::192.168.1.101::INSTR"]
set dmm [rsvisa::OpenResource $rm "GPIB::10::INSTR"]

# Query all instruments
puts "Scope: [rsvisa::Idn $scope]"
puts "Generator: [rsvisa::Idn $generator]"
puts "DMM: [rsvisa::Idn $dmm]"

# Configure generator
rsvisa::Write $generator "FREQ 1000\n"
rsvisa::Write $generator "VOLT 2.5\n"
rsvisa::Write $generator "OUTPut ON\n"

# Measure with DMM
set voltage [rsvisa::WriteRead $dmm "MEASure:VOLTage:DC?\n"]
puts "Measured: $voltage V"

# Capture with scope
set waveform [rsvisa::WriteReadBin $scope "WAVeform:DATA?\n"]

# Cleanup
rsvisa::CloseResource $scope
rsvisa::CloseResource $generator
rsvisa::CloseResource $dmm
```

## Documentation

Full documentation is available in the man page:

```bash
man rsvisa
```

Or view the source documentation in `doc/rsvisa.n`.

## Testing

The test suite uses the Tcl `tcltest` framework:

```bash
# Run all tests
make test

# Or run tests manually
cd tests
tclsh all.tcl
```

**Note**: Full integration tests require actual VISA hardware or a VISA simulator. Basic smoke tests verify package loading and command registration.

## Troubleshooting

### "Can't find package rsvisa"

Ensure the extension is installed in a directory that Tcl searches. Add the installation directory to `auto_path`:

```tcl
lappend auto_path /path/to/rsvisa1.0
package require rsvisa
```

### "VI_ERROR_RSRC_NFOUND"

- Verify the instrument is powered on and connected
- Check the resource URI format
- Use R&S VISA tools to discover available resources
- Ensure firewalls allow instrument communication (for LAN)

### Compilation Errors

- Verify R&S VISA headers are in the include path
- Ensure MinGW-w64 is used (not legacy MinGW)
- Check that C++11 support is enabled
- Verify Tcl development files are installed

### Runtime Errors on Windows

- Ensure `RsVisa32.dll` is in the system PATH
- Check that the correct MinGW-w64 runtime DLLs are available
- Verify Tcl 8.6 stubs library is present

## License

Copyright (c) 2025 TecLab
Copyright (c) 2002 IVI Foundation

This software is licensed under the IVI Foundation License. See the `LICENSE` file for complete terms.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software to use, copy, modify, merge, publish, and distribute the software, subject to the terms in the LICENSE file.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.

## Contributing

Contributions are welcome! Please ensure:

- Code follows the existing style
- Changes are tested on both Windows and Linux
- Documentation is updated as needed
- Commit messages are descriptive

## Links

- **IVI Foundation**: [https://www.ivifoundation.org/](https://www.ivifoundation.org/)
- **R&S VISA Downloads**: [https://www.rohde-schwarz.com/de/driver-pages/fernsteuerung/3-visa-und-tools_231388.html](https://www.rohde-schwarz.com/de/driver-pages/fernsteuerung/3-visa-und-tools_231388.html)
- **R&S VISA Application Note**: [https://www.rohde-schwarz.com/fi/applications/r-s-visa-application-note_56280-148812.html](https://www.rohde-schwarz.com/fi/applications/r-s-visa-application-note_56280-148812.html)
- **VISA Specification**: Available from IVI Foundation website

## Version History

- **1.0.0** (2025) - Initial release
  - 13 core VISA commands
  - R&S VISA integration
  - TEA build system
  - Tcl stubs support
  - Cross-platform support (Windows/Linux)
  - MSYS2/MinGW-w64 compatibility
