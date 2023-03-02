add_library(sdcard
  sdcard.h
  sdio.pio
  sdio.h sdio.cpp
  sd_card_sdio.cpp
  sd_card_spi.cpp
)

pico_generate_pio_header(sdcard ${CMAKE_CURRENT_LIST_DIR}/sdio.pio)

target_link_libraries(sdcard PRIVATE hardware_pio
                             PRIVATE hardware_dma
                             PRIVATE hardware_spi
                             PRIVATE hardware_gpio
                             PUBLIC sdfat
)

target_include_directories(sdcard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
