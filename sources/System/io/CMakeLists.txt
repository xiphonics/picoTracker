add_library(system_io
  Status.h Status.cpp
)

target_include_directories(system_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
