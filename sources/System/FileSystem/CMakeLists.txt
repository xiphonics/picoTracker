add_library(system_filesystem
  PicoFileSystem.h PicoFileSystem.cpp
)

target_link_libraries(system_filesystem 
                                          PUBLIC application_utils
                                          PUBLIC system_errors
                                          PUBLIC sdfat
                                          PUBLIC platform_sdcard
                                          PUBLIC etl
)


target_include_directories(system_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
