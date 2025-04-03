add_library(platform_midi
  picoTrackerMidiService.h picoTrackerMidiService.cpp
  picoTrackerUSBMidiOutDevice.h picoTrackerUSBMidiOutDevice.cpp
  picoTrackerMidiInDevice.h picoTrackerMidiInDevice.cpp
  picoTrackerMidiOutDevice.h picoTrackerMidiOutDevice.cpp
  picoTrackerUSBMidiInDevice.h picoTrackerUSBMidiInDevice.cpp
  
)

target_link_libraries(platform_midi PRIVATE pico_stdlib
                                            platform_usb
)

target_include_directories(platform_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

