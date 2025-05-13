add_library(crsid
  SID.cpp
)

target_include_directories(crsid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
