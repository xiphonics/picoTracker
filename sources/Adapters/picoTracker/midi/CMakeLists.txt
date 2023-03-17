add_library(platform_midi
  picoTrackerMidiService.h picoTrackerMidiService.cpp
  picoTrackerMidiDevice.h picoTrackerMidiDevice.cpp
)

target_link_libraries(platform_midi PUBLIC pico_stdlib
)

target_include_directories(platform_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

