add_library(platform_filesystem
  picoTrackerFileSystem.cpp
)

target_link_libraries(platform_filesystem PUBLIC
  system_filesystem
  system_errors
  sdfat
  platform_sdcard
  etl
)

target_include_directories(platform_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
