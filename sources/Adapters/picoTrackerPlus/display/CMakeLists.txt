add_library(platform_display
  font.h
  ili9341.h ili9341.c
  mode0.h mode0.c
#  mode1.h mode1.c
#  mode2.h mode1.c
)

target_link_libraries(platform_display PUBLIC pico_stdlib
                                       PUBLIC hardware_spi
)

target_include_directories(platform_display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
