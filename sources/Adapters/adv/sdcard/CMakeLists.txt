add_library(platform_sdcard
  diskio.c
  ff_gen_drv.c
  sd_diskio.c
)

target_link_libraries(platform_sdcard
  fatfs
)

target_include_directories(platform_sdcard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
