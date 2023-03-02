add_library(pico_utils
  utils.h utils.cpp
)

target_link_libraries(pico_utils PUBLIC pico_stdlib
)

target_include_directories(pico_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
