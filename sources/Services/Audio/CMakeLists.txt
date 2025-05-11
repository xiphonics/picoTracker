add_library(services_audio
  Audio.cpp
  AudioDriver.cpp
  AudioMixer.cpp
  AudioOut.cpp
  AudioOutDriver.cpp
)

target_link_libraries(services_audio PUBLIC
  services_time
  application_instruments
  application_player
)

target_include_directories(services_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
