add_library(system_filesystem
  FileSystem.h FileSystem.cpp
)

target_link_libraries(system_filesystem PUBLIC application_utils
)

target_include_directories(system_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
