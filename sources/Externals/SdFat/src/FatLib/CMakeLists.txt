add_library(sdfat_fatlib
  FatFile.h FatFile.cpp
  FatPartition.h FatPartition.cpp
  FatFileLFN.cpp
  FatFileSFN.cpp
  FatName.cpp
  FatVolume.h FatVolume.cpp
  FatFilePrint.cpp
)

target_link_libraries(sdfat_fatlib PUBLIC pico_stdlib)

target_include_directories(sdfat_fatlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
