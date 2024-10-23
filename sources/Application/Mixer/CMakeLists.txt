add_library(application_mixer
  MixBus.h MixBus.cpp
  MixerService.h MixerService.cpp
)

target_link_libraries(application_mixer PUBLIC pico_stdlib
                                        PUBLIC application_audio
                                        PUBLIC sdfat
                                        PUBLIC platform_sdcard
)

target_include_directories(application_mixer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
