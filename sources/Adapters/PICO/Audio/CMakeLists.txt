add_library(hw_audio
  PICOAudio.h PICOAudio.cpp
  PICOAudioDriver.h PICOAudioDriver.cpp
)

target_link_libraries(hw_audio PUBLIC pico_stdlib
                               PUBLIC pico_audio_headers
                               PUBLIC pico_audio_i2s
                               PUBLIC pico_utils
)

target_include_directories(hw_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
