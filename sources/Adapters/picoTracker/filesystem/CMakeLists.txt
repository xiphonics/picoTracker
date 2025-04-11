add_library(platform_filesystem
  picoTrackerFileSystem.h picoTrackerFileSystem.cpp
)

target_link_libraries(platform_filesystem 
                     PUBLIC system_filesystem
                     PUBLIC system_errors
                     PUBLIC sdfat
                     PUBLIC platform_sdcard
                     PUBLIC etl
                     pico_multicore
)

target_include_directories(platform_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
