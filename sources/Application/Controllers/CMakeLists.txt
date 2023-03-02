add_library(application_controllers
  ControlRoom.h ControlRoom.cpp
)

target_link_libraries(application_controllers PUBLIC services_controllers
)

target_include_directories(application_controllers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
