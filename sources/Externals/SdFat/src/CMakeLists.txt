add_library(sdfat
  FreeStack.h FreeStack.cpp
  MinimumSerial.h MinimumSerial.cpp
  RingBuf.h
  SdFat.h
  SdFatConfig.h
  sdios.h
)

target_link_libraries(sdfat PUBLIC sdfat_fslib
)

#target_compile_definitions(sdfat PRIVATE SPI_DRIVER_SELECT=3 SD_CHIP_SELECT_MODE=2 ENABLE_DEDICATED_SPI=1 HAS_SDIO_CLASS)
#add_compile_definitions(SPI_DRIVER_SELECT=3)
#set(SPI_DRIVER_SELECT 3)

#add_definitions(SD_CHIP_SELECT_MODE=2)
#set(SD_CHIP_SELECT_MODE "2")
#add_definitions(ENABLE_DEDICATED_SPI=1)
#set(ENABLE_DEDICATED_SPI "1")
#add_definitions(-DHAS_SDIO_CLASS)

target_include_directories(sdfat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}  # Add in order to create a new root for this subproject
)

add_subdirectory(common)
add_subdirectory(SdCard)
add_subdirectory(FsLib)
add_subdirectory(ExFatLib)
add_subdirectory(FatLib)
