add_library(foundation
  Observable.cpp
)

target_link_libraries(foundation PUBLIC system_console
)

target_include_directories(foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(Variables)
add_subdirectory(Services)
