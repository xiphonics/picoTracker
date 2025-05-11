add_library(system_errors
  Result.cpp
)

target_include_directories(system_errors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
