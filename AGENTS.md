# Code Assistant Context

This document provides context for assistants to understand the picoTracker project 

## Project Overview

picoTracker is a hardware music tracker. The firmware is a modified and greatly extended version of LittleGPTracker 

The project is written in C++20 and uses CMake for building. It has a modular
architecture, with different components separated into directories like
`Application`, `Services`, `System`, and `UIFramework` 

The codebase is cross platform, supporting firmware for both the original RP2040
based "pico" model and the STM32H7 based "Advance" (adv) model. 
The platform specific code for each lives under the respective Adaptors sub directories 

**Key Technologies:**

- **C++20:** The primary programming language 
- **CMake:** The build system used to compile the project 
- **Raspberry Pi Pico SDK:** Used for building the firmware for the Pico 
- **FreeRTOS:** Used for the "picoTracker Advance" board 
- **ETL (Embedded Template Library):** A C++ template library for embedded systems 
- **Sdfat:** FAT file system module for the pico model 
- **FatFs:** FAT file system module for the Advance (adv)
- **tinyusb:** An open-source USB host/device stack 

## Building and Running

The project is built using CMake  Here are the general steps to build the
project:

1  **Clone the repository:**
   ```bash
   git clone https://github.com/democloid/picoTracker
   cd picoTracker
   git submodule update --init --recursive
   ```

3. **Create a build directory:**
   ```bash
   mkdir build
   cd build
   ```

4. **Run CMake for pico:**
   ```bash
   PICO_SDK_PATH=../sources/Externals/pico-sdk cmake ../sources
   ```

5. **Build the project:**
   ```bash
   make -j$(nproc)
   ```

**To create a debug build for pico:**

```bash
PICO_SDK_PATH=../sources/Externals/pico-sdk cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ../sources/
```

## Development Conventions

- **Coding Style:** The project follows the
  [LLVM C++ Style Guide](https://google.github.io/styleguide/cppguide.html) 
  Code formatting is enforced by `clang-format`. Before considering code ready, run `clang-format` to ensure your code adheres to the style guide 
  We do not use variable name prefixes like "g_" or "k"
  We always use fixed width integer types like uint32_t or int16_t *never* just int or char 
  We always use designated struct initialisers *never* old style C ones 
- **`printf` usage:** The `printf` family of functions should not be used. Instead, use the `nanoprintf` library functions found in `nanoprintf.h`. 
  For debug logging, always use the `Trace` class 
- **Debugging:** The recommended way to debug is by using a `picoProbe` for pico, stlink for Avd and `OpenOCD` for both. 
  The `docs/DEV.md` file provides detailed instructions on how to set up a debugging environment 
- **CI:** GitHub Actions are used for continuous integration

## Key Files and Directories

- `sources/`: Contains the C++ source code for the project
- `sources/CMakeLists.txt`: The main CMake file that defines the project structure, dependencies, and build process
- `sources/Adapters/`: Contains the platform-specific code for the Pico and the Adv board
- `sources/Application/`: Contains the main application logic
- `sources/Services/`: Contains services used by the application, such as audio and MIDI
- `sources/Externals/`: Contains external libraries and dependencies
- `docs/`: Contains developer documentation for the project, including build guides, a developer guide
- `usermanual` : Contains the user manual in markdown format built with picosite SSG
- `README.md`: The main README file for the project
- `.clang-format`: The configuration file for `clang-format`
