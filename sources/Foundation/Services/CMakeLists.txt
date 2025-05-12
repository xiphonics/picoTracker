add_library(foundation_services
  Service.cpp
  ServiceRegistry.cpp
  SubService.cpp
)

target_include_directories(foundation_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
