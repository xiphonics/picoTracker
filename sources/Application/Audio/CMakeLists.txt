add_library(application_audio
  AudioFileStreamer.h AudioFileStreamer.cpp
  DummyAudioOut.h DummyAudioOut.cpp
)

target_link_libraries(application_audio PUBLIC system_process
                                        PUBLIC services_time
                                        PUBLIC sdfat
                                        PUBLIC platform_sdcard
)

target_include_directories(application_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
