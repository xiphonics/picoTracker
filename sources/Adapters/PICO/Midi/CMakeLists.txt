add_library(pico_midi
  PICOMidiService.h PICOMidiService.cpp
  PICOSerialMidiDevice.h PICOSerialMidiDevice.cpp
)

target_link_libraries(pico_midi PUBLIC pico_stdlib
)

target_include_directories(pico_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

