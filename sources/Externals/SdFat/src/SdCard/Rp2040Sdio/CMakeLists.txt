add_library(sdfat_rp2040
  PioSdioCard.h
  Rp2040SdioConfig.h
)

pico_generate_pio_header(sdfat_rp2040 ${CMAKE_CURRENT_LIST_DIR}/PioSdioCard.pio)

target_link_libraries(sdfat_rp2040 PUBLIC pico_stdlib
)

target_include_directories(sdfat_rp2040 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
