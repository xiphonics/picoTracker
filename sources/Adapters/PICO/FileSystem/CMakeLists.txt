add_library(pico_filesystem
  PICOFileSystem.h PICOFileSystem.cpp
)

target_link_libraries(pico_filesystem PUBLIC system_filesystem
                                       PUBLIC system_errors
                                       PUBLIC sdfat
                                       PUBLIC sdcard
)

target_include_directories(pico_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

