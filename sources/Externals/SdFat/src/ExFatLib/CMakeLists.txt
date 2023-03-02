add_library(sdfat_exfatlib
  ExFatFileWrite.cpp
  ExFatPartition.h ExFatPartition.cpp
  ExFatFile.h ExFatFile.cpp
  ExFatVolume.h ExFatVolume.cpp
  ExFatName.cpp
  ExFatFilePrint.cpp
)

target_link_libraries(sdfat_exfatlib PUBLIC sdfat_common)

target_include_directories(sdfat_exfatlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
