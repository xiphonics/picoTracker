add_library(platform_timer
  picoTrackerTimer.cpp
)

# target_link_libraries(platform_timer PUBLIC
#   system_timer
#   foundation
# )

target_link_libraries(platform_timer PUBLIC pico_sdk_bundle)

target_include_directories(platform_timer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${PROJECT_SOURCE_DIR}
)

