add_library(application_utils
  HexBuffers.cpp
  char.cpp
  fixed.cpp
  stringutils.cpp 
  DrawUtils.cpp
)

target_link_libraries(application_utils PUBLIC
  etl
)

target_include_directories(application_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
