add_library(application_utils
  HexBuffers.h HexBuffers.cpp
  char.h char.cpp
  fixed.h fixed.cpp
  randomnames.h
  stringutils.h stringutils.cpp 
)

target_link_libraries(application_utils
                                              PUBLIC sdfat
                                              PUBLIC platform_sdcard
                                              PUBLIC etl
                                              PUBLIC pico_stdlib
                                              PUBLIC pico_rand
)

target_include_directories(application_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
