add_library(application_persistency
  Persistent.h Persistent.cpp
  PersistencyService.h PersistencyService.cpp
#  PersistencyDocument.h PersistencyDocument.cpp
)

target_link_libraries(application_persistency PUBLIC foundation_services
#                                              PUBLIC yxml
                                              PUBLIC compression
)

target_include_directories(application_persistency PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
