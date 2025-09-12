add_library(remote_ui
  RemoteUIProtocol.cpp
)

target_include_directories(remote_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
