add_library(services_audio
  Audio.h Audio.cpp
  AudioDriver.h AudioDriver.cpp
  AudioMixer.h AudioMixer.cpp
  AudioModule.h
  AudioOut.h AudioOut.cpp
  AudioOutDriver.h AudioOutDriver.cpp
  AudioSettings.h
)

target_link_libraries(services_audio PUBLIC services_time
                                     PUBLIC application_instruments
                                     PUBLIC application_player)

target_include_directories(services_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
