add_library(platform_mutex
  advMutex.cpp
)

#target_link_libraries(platform_mutex
#)

target_include_directories(platform_mutex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
