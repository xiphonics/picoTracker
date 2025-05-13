add_library(yxml
  yxml.c
)

target_include_directories(yxml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
