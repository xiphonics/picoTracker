add_library(sdfat_sdcard
  SdCard.h
  SdSpiCard.h SdSpiCard.cpp
  SdCardInfo.h SdCardInfo.cpp
)

target_link_libraries(sdfat_sdcard PUBLIC pico_utils
)

target_include_directories(sdfat_sdcard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
