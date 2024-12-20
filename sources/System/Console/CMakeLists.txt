add_library(system_console
  nanoprintf.h
  Trace.h Trace.cpp
  n_assert.h n_assert.cpp
)
target_link_libraries(system_console PRIVATE pico_stdlib
)

target_include_directories(system_console PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
