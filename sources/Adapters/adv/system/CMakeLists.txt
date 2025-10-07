add_library(platform_system
  advSystem.cpp
  input.cpp
  critical_error_message.c
  advSamplePool.cpp
  BatteryGauge.cpp
  syscalls.c
  power.cpp
)

target_link_libraries(platform_system PUBLIC
  uiframework_simplebaseclasses
  platform_timer
  platform_audio
  services_audio
  platform_midi
  platform_gui
  platform_display
  platform_filesystem
  etl
  INTERFACE system_system
)

target_include_directories(platform_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
