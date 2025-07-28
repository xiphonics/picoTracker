add_library(platform
  platform.cpp
)

target_link_libraries(platform PUBLIC pico_sdk_bundle)

target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
