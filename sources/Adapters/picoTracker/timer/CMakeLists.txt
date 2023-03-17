add_library(platform_timer
  picoTrackerTimer.h picoTrackerTimer.cpp
)

target_link_libraries(platform_timer PUBLIC pico_stdlib
                                     PUBLIC system_timer
                                     PUBLIC foundation
)

target_include_directories(platform_timer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${PROJECT_SOURCE_DIR}
)

