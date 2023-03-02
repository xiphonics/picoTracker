add_library(foundation_services
  Service.h Service.cpp
  ServiceRegistry.h ServiceRegistry.cpp
  SubService.h SubService.cpp
)

target_include_directories(foundation_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
