set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Profiler.cpp
)

add_library(Performance ${SOURCES})

target_link_libraries(Performance PUBLIC 
    pico_stdlib
)

target_include_directories(Performance PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
