add_library(profiler Profiler.cpp)

target_include_directories(profiler PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(${PROJECT_SOURCE_DIR})
