add_library(yxml
  yxml.h yxml.c
)

#target_link_libraries(yxml PUBLIC system_filesystem
#)

target_include_directories(yxml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
