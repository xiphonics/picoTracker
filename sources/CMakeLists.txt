cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoTracker)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Make this a picoTracker build within the project
add_definitions(-DPICOBUILD)

# For debugging purposes, print all mallocs
# add_definitions(-DPICO_DEBUG_MALLOC)
# add_definitions(-DPICOSTATS)
# add_definitions(-DALL_MALLOC)
# add_definitions(-DSHOW_MEM_USAGE)
add_definitions(-DDISABLESF)
# Enable SDIO - this setting affects code in SdFat library as well as the project
# This consumes ~7k+ RAM and has some performance problems ATM
# Also enable HAS_SDIO_CLASS in SdFatConfig.h
# Switching to this mode may require full pico reset (why?)
# add_definitions(-DSD_SDIO)
# Enable Streaming from SD, bad performance for the moment
# add_definitions(-DSD_STREAMING)
# Enable loading samples into Flash
add_definitions(-DLOAD_IN_FLASH)
# Disable MIDI. Enable in order to use UART0 as stdio for debugging
# add_definitions(-DDUMMY_MIDI)
# Disable feedback for sample instruments. Due to low memory in the PICO
# there is very low chance to bring this back
add_definitions(-DDISABLE_FEEDBACK)
# Disable exit dialogs. Eventually this could send device to dormant mode
# but probably unnecessary since it's easier and safe to just turn off
add_definitions(-DNO_EXIT)

# Initialize the SDK
pico_sdk_init()

add_compile_options(
  -Werror
  -Wall
  -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
  # TODO: SHOULD FIX
  -Wno-int-to-pointer-cast
)

add_subdirectory(Adapters)
add_subdirectory(UIFramework)
add_subdirectory(System)
add_subdirectory(Application)
add_subdirectory(Externals)
add_subdirectory(Services)
add_subdirectory(Foundation)

set_property(TARGET picoTracker APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")


target_compile_definitions(picoTracker PUBLIC
#	PICO_HEAP_SIZE=204800
  PICO_DEBUG_MALLOC_LOW_WATER=1000
  PICO_MALLOC_PANIC=0
  PICO_STACK_SIZE=4096
  PICO_CORE1_STACK_SIZE=4096 # Safe value. 2K seems to be enough. Is it for all use cases?
  PICO_USE_STACK_GUARDS
)

#if (SD_SDIO)
#MESSAGE(STATUS "SDIO enabled")
#target_compile_definitions(picoTracker PUBLIC HAS_SDIO_CLASS=1)
#endif()

# Debug output with USB uses +6K memory
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# This is the default, but better to make it explicit
pico_set_float_implementation(${PROJECT_NAME} pico)
