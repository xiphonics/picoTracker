add_library(platform_system
  picoTrackerSystem.cpp
  picoTrackerEventQueue.cpp
  input.cpp
  critical_error_message.c
  picoTrackerSamplePool.cpp
)

target_link_libraries(platform_system INTERFACE system_system PUBLIC
  uiframework_simplebaseclasses
  platform_timer
  platform_audio
  services_audio
  platform_midi
  platform_gui
  platform_display
  etl
  platform_usb
)


target_include_directories(platform_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
