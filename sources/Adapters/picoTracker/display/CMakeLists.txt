add_library(platform_display
  ili9341.c
  mode0.c
)

target_include_directories(platform_display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
