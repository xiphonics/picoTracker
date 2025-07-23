add_library(platform_display
  ili9341.c
  chargfx.c
  picoBitmapGraphics.cpp
)

target_include_directories(platform_display PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries(platform_display PUBLIC
    pico_stdlib
    hardware_spi
)
