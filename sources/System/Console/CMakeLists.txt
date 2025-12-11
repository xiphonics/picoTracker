add_library(system_console
  Trace.cpp
  n_assert.cpp
  AllocTracker.cpp
)

target_include_directories(system_console PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
