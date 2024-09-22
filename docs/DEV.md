# Development
If you'd like to make changes and develop for the picoTracker, here are guidelines about how to do so.

picoTracker is based on the Raspberry Pi Pico and it's built around the [C SDK](https://www.raspberrypi.com/documentation/pico-sdk/) of the platform.
The only requirement is to install the toolchain as explained in [Getting started](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) document, this will depend on each platform. It's essentially: Basic build tools (build-essentials in Debian based distros), cmake and arm cross compiler.

## Building optimized version
Assuming a unix style operating system (Linux, OSX). Below example assumes 8 thread processor on your build machine, adjust ```-j``` flag on the ```make``` command to match your platform.
```
~ % git clone https://github.com/democloid/picoTracker
~ % cd picoTracker
picoTracker % git submodule update --init --recursive
picoTracker % mkdir build
picoTracker/build % cd build
picoTracker/build % PICO_SDK_PATH=../sources/Externals/pico-sdk cmake ../sources
picoTracker/build % make -j8
```

You can also set a specific path that contains the toolchain (gcc etc) with:
```
PICO_SDK_PATH=../sources/Externals/pico-sdk cmake -DPICO_TOOLCHAIN_PATH=/path/to/your/arm-none-eabi-toolchain  ../sources
```

Subsequent builds just need the make command. If anything changed on the CMakeLists files, cmake will update itself.

## Building Debug version
To create a debug build, you have to replace the ```cmake``` step on the previous step with:
```picoTracker/build % PICO_SDK_PATH=../sources/Externals/pico-sdk cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ../sources/```

## Testbench for development
While you can perform changes by building and copying the resulting binary onto the Pico using USB, this will be extremely slow and painful. A better setup would be to use a [picoProbe](https://github.com/raspberrypi/picoprobe) and use [OpenOCD](https://openocd.org/) in order to iterate quickly while you're developing.

## Running OpenOCD
(Building OCD is out of scope of this document)
```openocd/tcl % ../src/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "bindto 0.0.0.0"```

The ```bindto``` command can be omitted if you're developing locally. Unfortunatelly I develop on an M1 Mac, where gdb-multiarch is not available, so I connect from a remote Linux machine.

## Configuring the build for serial output of logs
The main CMake configuration file is located at ```picoTracker/sources/CMakeLists.txt```, there are two options to enable serial output, USB or UART0. To configure either of these options, set 1 to the configuration option you desire:
```
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
```

Couple of things to keep in mind:
* If you decide to use USB passthrough as your serial output, this will consume 6KB of memory which is a pretty scarse resource, and given that this project uses a lot of dynamic allocations, you may find memory issues in the form of hardfaults which may be hard to debug. It *SHOULD* be ok, but be warned.
* Another annoying thing about using USB for serial output is that the USB port is not immediately available upon boot, so you cannot see the initial messages.
* Yet annother annoyence about USB is that you'll have to use two USB ports, one for picoProbe and another one for your serial console to your actual picoTracker Pico.
* If you choose to use UART0 for serial output, there's an annoyance too. In the final build I decided to simplify input and use a GPIO per key, rather than the matrix I was using during development. This means that UART0 is now used for MIDI in/out, so this port is not free. There are two requirements here:
 * You'll have to figure out the best way to connect your serial output into your picoTracker. Probably the cleanest, but overkill, would be to hack some MIDI cable and plug directly into the MIDI port.
 * You'll have to disable MIDI (so this option will not work for MIDI development/troubleshooting) by enabling ```add_definitions(-DDUMMY_MIDI)``` in ```picoTracker/sources/CMakeLists.txt```
 
## Connecting to your serial output
Use your serial communications program of choice to connect to the serial console of the pico.
If using USB serial (where the usbmodem device is the Pico USB):
```~ % minicom -D /dev/tty.usbmodem2101```

If using picoProbe serial though OpenOCD (where the usbmodem device is the picoProbe USB)
```~ % minicom -D /dev/tty.usbmodem1101 localhost:4444```

## Loading code and running it using gdb
Replace ```localhost``` with whatever your host running OpenOCD is

```gdb-multiarch <PATH_TO_BUILD>/Adapters/picoTracker/main/picoTracker.elf --eval-command="target extended-remote localhost:3333" --eval-command="load" --eval-command="monitor reset init" --eval-command="continue"```

This command should load your code into the pico, reset it and run the program. You can go ahead now and use any GDB functionality you may like, or not use it at all other than to load and run your code.


## Github Actions CI

The actions CI workflows include a check for the projects specific code formatting style and building the firmware image. It may be useful to run these actions on a local machine, in which case the [act](https://nektosact.com/installation/index.html) could be useful. Note to use `act` you nee to have docker already installed as well.

## MIDI Development

### Debugging MIDI TRS (serial) output

NOTE: The original design picoTracker shares the same UART for both serial output for debugging and TRS MIDI output so you **MUST** enable `DDUMMY_MIDI` and disable `pico_enable_stdio_uart` in the CMakeLists.txt file to use the UART for debugging!

A helpful tool for debugging MIDI output is [ShowMIDI](https://github.com/gbevin/ShowMIDI) or for more low level debugging you may want to use a [BusPirate](http://dangerousprototypes.com/docs/Bus_Pirate).

### USB MIDI output

TODO