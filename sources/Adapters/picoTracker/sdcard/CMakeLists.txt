add_library(platform_sdcard
  sdio.cpp
  sd_card_sdio.cpp
)

pico_generate_pio_header(platform_sdcard ${CMAKE_CURRENT_LIST_DIR}/sdio.pio)

target_link_libraries(platform_sdcard PUBLIC
  sdfat
)

target_include_directories(platform_sdcard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
