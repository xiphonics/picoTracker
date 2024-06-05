add_library(swdloader
  swdloader.h swdloader.cpp
)

target_link_libraries(swdloader PUBLIC pico_stdlib
)

target_include_directories(swdloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
