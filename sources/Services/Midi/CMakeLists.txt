add_library(services_midi
  MidiChannel.h MidiChannel.cpp
  MidiEvent.h MidiEvent.cpp
  MidiInDevice.h MidiInDevice.cpp
  MidiInMerger.h MidiInMerger.cpp
  MidiMessage.h
  MidiOutDevice.h MidiOutDevice.cpp
  MidiService.h MidiService.cpp
)

target_link_libraries(services_midi PUBLIC services_controllers 
  PRIVATE sdfat # need this due transitive dep on XML lib coming from the Config class dep
)

target_include_directories(services_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
