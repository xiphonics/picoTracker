add_library(services_time
  TimeService.cpp
)

target_link_libraries(services_time PUBLIC
  system_system
  foundation
)

target_include_directories(services_time PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

