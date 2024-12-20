add_library(platform_power
  # bq27441-g1.h
  rosc.h rosc.cpp
  rpi_helpers.h rpi_helpers.cpp
  power_button.h power_button.cpp
)

target_link_libraries(platform_power
  PUBLIC pico_stdlib
  system_console
  platform
  hardware_clocks
  hardware_irq
  pico_aon_timer
  hardware_powman
)

target_include_directories(platform_power PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
