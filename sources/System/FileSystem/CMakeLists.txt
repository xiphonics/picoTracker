add_library(system_filesystem
)

target_link_libraries(system_filesystem PUBLIC
  application_utils
  system_errors
  etl
)


target_include_directories(system_filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
