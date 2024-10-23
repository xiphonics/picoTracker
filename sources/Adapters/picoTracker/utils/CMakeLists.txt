add_library(platform_utils
  utils.h utils.cpp
)

target_link_libraries(platform_utils PUBLIC pico_stdlib
  PUBLIC sdfat
)

target_include_directories(platform_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
