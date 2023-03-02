add_library(compression
  lz.h lz.c
)

target_include_directories(compression PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
