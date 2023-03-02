add_library(application_utils
  HexBuffers.h HexBuffers.cpp
  StringTokenizer.h
  char.h char.cpp
  fixed.h fixed.cpp
  wildcard.h wildcard.cpp
)

target_include_directories(application_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
