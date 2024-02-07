add_library(platform_audio
  picoTrackerAudio.h picoTrackerAudio.cpp
  picoTrackerAudioDriver.h picoTrackerAudioDriver.cpp
)

pico_generate_pio_header(platform_audio ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)
pico_generate_pio_header(platform_audio ${CMAKE_CURRENT_LIST_DIR}/audio_lineout_i2s.pio)

target_link_libraries(platform_audio PUBLIC pico_stdlib
                                     PUBLIC pico_multicore
                                     PUBLIC platform_utils
                                     PUBLIC hardware_gpio
                                     PUBLIC hardware_dma
                                     PUBLIC hardware_clocks
                                     PUBLIC hardware_irq
                                     PUBLIC hardware_pio
)

target_include_directories(platform_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
