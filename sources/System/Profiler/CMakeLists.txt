add_library(profiler Profiler.cpp)

target_include_directories(profiler PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(profiler PUBLIC pico_sdk_bundle)

include_directories(${PROJECT_SOURCE_DIR})
