add_library(services_midi
  MidiEvent.cpp
  MidiInDevice.cpp
  MidiOutDevice.cpp
  MidiService.cpp
  MidiNoteTracker.cpp
)

target_include_directories(services_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
