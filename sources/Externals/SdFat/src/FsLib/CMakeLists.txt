add_library(sdfat_fslib
  FsVolume.h FsVolume.cpp
  FsFile.h FsFile.cpp
  FsNew.h FsNew.cpp
)

target_link_libraries(sdfat_fslib PUBLIC sdfat_exfatlib
                                  PUBLIC sdfat_fatlib
                                  )

target_include_directories(sdfat_fslib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
