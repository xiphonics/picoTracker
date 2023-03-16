add_library(platform
  gpio.h
  platform.h platform.cpp
)

target_link_libraries(platform PUBLIC pico_stdlib
                               PUBLIC hardware_gpio 
                               PUBLIC hardware_spi
                               PUBLIC hardware_clocks
                               PUBLIC hardware_pll
                               PUBLIC hardware_uart
)

target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
