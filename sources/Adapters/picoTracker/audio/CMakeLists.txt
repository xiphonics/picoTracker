add_library(platform_audio
  picoTrackerAudio.cpp
  picoTrackerAudioDriver.cpp
)

pico_generate_pio_header(platform_audio ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)

target_link_libraries(platform_audio PUBLIC
  platform_utils
)

target_include_directories(platform_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
