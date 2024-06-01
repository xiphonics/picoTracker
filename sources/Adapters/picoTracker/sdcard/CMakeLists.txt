add_library(platform_sdcard
  sdcard.h
  sdio.pio
  sdio.h sdio.cpp
  sd_card_sdio.cpp
)

pico_generate_pio_header(platform_sdcard ${CMAKE_CURRENT_LIST_DIR}/sdio.pio)

target_link_libraries(platform_sdcard PRIVATE hardware_pio
                                      PRIVATE hardware_dma
                                      PRIVATE hardware_spi
                                      PRIVATE hardware_gpio
                                      PUBLIC sdfat
)

target_include_directories(platform_sdcard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
