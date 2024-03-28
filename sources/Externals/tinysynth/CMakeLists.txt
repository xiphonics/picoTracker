add_library(tinysynth
tinysynth.cpp
tinysynth.h
fixed.h
)

target_link_libraries(tinysynth PUBLIC pico_stdlib
)

target_include_directories(tinysynth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})