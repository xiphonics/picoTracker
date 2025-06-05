cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pico-sdk)
# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoTracker)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# For debugging purposes, print all mallocs
# add_definitions(-DPICO_DEBUG_MALLOC)
# add_definitions(-DPICOSTATS)
# add_definitions(-DALL_MALLOC)
# add_definitions(-DSHOW_MEM_USAGE)
# Perform a benchmark of SD card using SDIO oin startup. Can be removed once performance issues have been solved
# add_definitions(-DSDIO_BENCH)
# Use USB for UI: keyboardinput & Display output. USB Uart MUST also be enabled
# below.
# add_definitions(-DUSB_REMOTE_UI)
#disabled for now as USB **input** is not yet fully implemented
#add_definitions(USB_REMOTE_UI_INPUT)
# enable basic control REPL over serial port. NOTE: uses extra ~112bytes of *stack*
# add_definitions(-DSERIAL_REPL)

# enable profiling
# add_definitions(-DENABLE_PROFILING)

# ST7789 controller based LCD instead of a ILI9341 controller
# for CI builds this will be set via command line
if(NOT USE_LCD)
    set(USE_LCD LCD_ST7789)
endif()
message(USE_LCD="${USE_LCD}")
add_compile_definitions(${USE_LCD})
# add_compile_definitions(LCD_ST7789)


# Needed for ETL to pick up our etl_profile.h
include_directories("config/")

# platform specific implementations
include_directories("Adapters/picoTracker/platform/")

# Enable ETL debug mode only for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(ETL_DEBUG)
endif()

# Initialize the SDK
pico_sdk_init()

link_libraries(
  pico_stdlib
  pico_sync
  pico_multicore
  pico_rand
  pico_unique_id
  tinyusb_board
  tinyusb_device
  hardware_pio
  hardware_irq
  hardware_clocks
  hardware_dma
  hardware_gpio
  hardware_spi
  hardware_watchdog
  hardware_vreg
  hardware_uart
  hardware_pll
  hardware_adc
  hardware_flash
)

add_subdirectory(Adapters/picoTracker)
add_subdirectory(Externals/SdFat)

add_compile_options(
  -Werror
  -Wall
  -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
  -fstack-usage
  -fcallgraph-info=su # include above info in callgraph
  -Wstack-usage=2048 # allows high watermark at the time this option was added TODO: reduce this for safety and/or reduce stack size
  # TODO: SHOULD FIX
  -Wno-int-to-pointer-cast

  # braids
  -Wno-unused-variable
  -Wno-array-bounds
)

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
  PICO_FLASH_SIZE_BYTES=16*1024*1024
  # workaround for slow init crystal on some boards see: https://github.com/raspberrypi/pico-sdk/pull/457
  PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
)

target_compile_definitions(picoTracker PUBLIC
  PICO_DEFAULT_UART=1
  PICO_DEFAULT_UART_TX_PIN=24
  PICO_DEFAULT_UART_RX_PIN=25
)

# Debug output with USB uses +6K memory
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# This is the default, but better to make it explicit
pico_set_float_implementation(${PROJECT_NAME} pico)
