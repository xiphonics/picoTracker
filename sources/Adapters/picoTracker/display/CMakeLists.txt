add_library(platform_display
  ili9341.c
  chargfx.c
  picoBitmapGraphics.cpp
)

target_link_libraries(platform_display PUBLIC pico_sdk_bundle)

target_include_directories(platform_display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                ${CMAKE_SOURCE_DIR}
)
