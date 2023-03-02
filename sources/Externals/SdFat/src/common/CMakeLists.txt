add_library(sdfat_common
  PrintBasic.h PrintBasic.cpp
  SysCall.h
  FsBlockDevice.h
  FsCache.h FsCache.cpp
  FsDateTime.h FsDateTime.cpp
  FsUtf.h FsUtf.cpp
  FmtNumber.h FmtNumber.cpp
)

target_link_libraries(sdfat_common PUBLIC sdfat_sdcard)

target_include_directories(sdfat_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(${PROJECT_SOURCE_DIR})
