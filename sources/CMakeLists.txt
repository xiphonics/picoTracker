cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json")

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

  # stm HAL
  -Wno-unused-parameter
  -Wno-missing-field-initializers
)

if (NOT DEFINED ADV)
  set(ADV false)
endif()

# pico
if (NOT ${ADV})
  set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pico-sdk)
  # Pull in SDK (must be before project)
  include(pico_sdk_import.cmake)

  project(picoTracker)

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
  # Enable loading samples into Flash
  add_definitions(-DLOAD_IN_FLASH)
  # Use USB for UI: keyboardinput & Display output. USB Uart MUST also be enabled
  # below.
  add_definitions(-DUSB_REMOTE_UI)
  # define to use battery level as percentage instead of battery level as "+" bars
  #add_definitions(-DBATTERY_LEVEL_AS_PERCENTAGE)

  #disabled for now as USB **input** is not yet fully implemented
  #add_definitions(USB_REMOTE_UI_INPUT)
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

  include_directories("Adapters/picoTracker/platform/")

  # Initialize the SDK
  pico_sdk_init()

  add_subdirectory(sdk_bundle)

#   # Link pico
#   link_libraries(
#     pico_stdlib
#     pico_sync
#     pico_multicore
#     pico_rand
#     tinyusb_board
#     tinyusb_device
#     hardware_pio
#     hardware_irq
#     hardware_clocks
#     hardware_dma
#     hardware_gpio
#     hardware_spi
#     hardware_watchdog
#     hardware_vreg
#     hardware_uart
#     hardware_pll
# #    sdfat
#     hardware_adc
#     hardware_flash
#     hardware_pwm
#   )

  add_subdirectory(Adapters/picoTracker)
  add_subdirectory(Externals/SdFat)

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
  pico_enable_stdio_uart(${PROJECT_NAME} 0)
  
  # This is the default, but better to make it explicit
  pico_set_float_implementation(${PROJECT_NAME} pico)

# Advance
else()
  message("picoTracker Advance Build")
  add_definitions(-DADV)
  # Enable basic control REPL over serial port. NOTE: uses extra ~112bytes of *stack*
  add_definitions(-DSERIAL_REPL)

  # Advance displays battery level as percentage instead of bars
  add_definitions(-DBATTERY_LEVEL_AS_PERCENTAGE)

  # Enable stats for RTOS
  #add_definitions(-DRTOS_STATS)

  # FreeRTOS goes before compiler definitions for cubemx since it
  # does it's own thing (doesn't compile after those definitions)
  add_library(freertos_config INTERFACE)
  target_include_directories(freertos_config SYSTEM
    INTERFACE
    Adapters/adv/config # The config file directory
    Adapters/adv/platform
  )

  target_compile_definitions(freertos_config
    INTERFACE
    projCOVERAGE_TEST=0)
  set(FREERTOS_HEAP 4)
  set(FREERTOS_PORT GCC_ARM_CM7 CACHE STRING "")

  # ------- cmake/gcc-arm-none-eabi.cmake START -------

  set(CMAKE_SYSTEM_NAME               Generic)
  set(CMAKE_SYSTEM_PROCESSOR          arm)

  set(CMAKE_C_COMPILER_ID GNU)
  set(CMAKE_CXX_COMPILER_ID GNU)

  # Some default GCC settings
  # arm-none-eabi- must be part of path environment
  set(TOOLCHAIN_PREFIX                arm-none-eabi-)

  set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)
  set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
  set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)
  set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}g++)
  set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
  set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

  set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
  set(CMAKE_EXEUTABLE_SUFFIX_C       ".elf")
  set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

  # MCU specific flags
  set(TARGET_FLAGS "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard ")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
  set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
  # TODO(democloid): reenable Wpedantic? (disabled due to tinyUSB
  #  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fdata-sections -ffunction-sections")

  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
  set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")
  
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
  endif()
  
  set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"${CMAKE_CURRENT_SOURCE_DIR}/Adapters/adv/STM32H743XX_FLASH.ld\"")
  # TODO: check if --specs=nosys.specs is needed, not part of original files, helps pass _gettimeofday error
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --specs=nano.specs --specs=nosys.specs")
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group")
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")
  # printf prints float (do we even need printf?)
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -u _printf_float")

  set(CMAKE_CXX_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lstdc++ -lsupc++ -Wl,--end-group")
  # ------- cmake/gcc-arm-none-eabi.cmake STOP -------

  project(picoTrackerAdvance C CXX ASM)

  add_subdirectory(Externals/FreeRTOS-Kernel)
  add_subdirectory(Externals/FatFs)
  add_subdirectory(Externals/tinyusb)
  
  # TODO: Add ETL here
  # cubemx and freertos is included on all targets
  link_libraries(stm32cubemx freertos_kernel)

#  set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config/lv_conf.h" CACHE STRING "Path to LVGL config")
#  set(LV_CONF_BUILD_DISABLE_DEMOS 1)
#  set(LV_CONF_BUILD_DISABLE_EXAMPLES 1)
#  add_subdirectory(Externals/lvgl)

  add_subdirectory(Adapters/adv)

endif()

# Enable ETL debug mode only for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(ETL_DEBUG)
endif()


add_subdirectory(UIFramework)
add_subdirectory(System)
add_subdirectory(Application)
add_subdirectory(Externals)
add_subdirectory(Services)
add_subdirectory(Foundation)

