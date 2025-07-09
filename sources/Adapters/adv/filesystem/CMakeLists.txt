add_library(platform_filesystem
  picoFileSystem.h picoFileSystem.cpp
)

target_link_libraries(platform_filesystem PUBLIC
  system_filesystem
  fatfs
)

target_include_directories(platform_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
