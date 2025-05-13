add_library(services_midi
  MidiChannel.cpp
  MidiEvent.cpp
  MidiInDevice.cpp
  MidiInMerger.cpp
  MidiOutDevice.cpp
  MidiService.cpp
)

target_link_libraries(services_midi PUBLIC
  services_controllers
)

target_include_directories(services_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
