add_library(platform_display
  display.c
  advBitmapGraphics.cpp
)

#target_link_libraries(platform_display
#)

target_include_directories(platform_display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
