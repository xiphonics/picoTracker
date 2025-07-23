add_library(fatfs
#  source/diskio.c
  source/ff.c
  source/ffsystem.c
  source/ffunicode.c
)

target_link_libraries(fatfs PUBLIC platform_sdcard
)

target_include_directories(fatfs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)

include_directories(${PROJECT_SOURCE_DIR})
