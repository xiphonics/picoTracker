add_library(hw_audio
  PICOAudio.h PICOAudio.cpp
  PICOAudioDriver.h PICOAudioDriver.cpp
)

pico_generate_pio_header(hw_audio ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)

target_link_libraries(hw_audio PUBLIC pico_stdlib
                               PUBLIC pico_utils
                               PUBLIC hardware_gpio
                               PUBLIC hardware_dma
                               PUBLIC hardware_clocks
                               PUBLIC hardware_irq
                               PUBLIC hardware_pio
)

target_include_directories(hw_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
