add_library(tinyxml2
  tinyxml2.cpp
  tinyxml2adapter.cpp
)

target_link_libraries(tinyxml2 PUBLIC
  system_filesystem
)

target_include_directories(tinyxml2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
