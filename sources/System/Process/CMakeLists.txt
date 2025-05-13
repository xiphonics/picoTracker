add_library(system_process
  SysMutex.cpp
)

target_include_directories(system_process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
