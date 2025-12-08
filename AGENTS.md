## Project Overview

picoTracker is a hardware music tracker firmware.
The codebase is cross platform, supporting RP2040 (pico) and STM32H7 Advancce (Adv) based device.

**Key Technologies:**

- C++20: Using GCC toolchain
- CMake: The build system
- Raspberry Pi Pico SDK: Used for building the firmware for the pico
- FreeRTOS: Used for the Adv
- ETL (Embedded Template Library): used in prefence to the standard C++ template library in this project
- Sdfat:FAT file system module for the pico only
- FatFs: FAT file system module for the Adv only
- tinyusb: USB stack used for both pico and Adv
- clang-format: enforces code formatting

## Building and Running

Look in docs/DEV.md for how to build the project and for general code style conventions used.

## Code Style & Conventions

* We do NOT use variable name prefixes like "g_" or "k"
* We always use fixed width integer types like uint32_t or int16_t *never* just int or char
* We always use designated struct initialisers *never* old style C ones
* Never use `printf` family of functions, instead use functions from `nanoprintf.h`
* For debug logging, only use the `Trace` class

## Key Files and Directories

- `sources/CMakeLists.txt`: the top level CMake file
- `sources/Adapters/`: the platform-specific code for pico and Adv
- `sources/Application/`: the main application logic
- `sources/Application/Views`: UI classes for the firmware
- `sources/Services/`: the services such as audio and MIDI
- `sources/Externals/`: external library dependencies