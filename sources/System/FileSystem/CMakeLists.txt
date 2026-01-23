add_library(system_filesystem INTERFACE)

target_link_libraries(system_filesystem INTERFACE
  application_utils
  etl
)


target_include_directories(system_filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
