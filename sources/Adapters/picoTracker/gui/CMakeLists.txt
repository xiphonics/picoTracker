add_library(platform_gui
  GUIFactory.cpp
  picoTrackerGUIWindowImp.cpp
  picoTrackerEventManager.cpp
  SerialDebugUI.cpp
  picoRemoteUI.cpp
)

target_link_libraries(platform_gui PUBLIC
  uiframework_simplebaseclasses
  system_console
  uiframework_basicdatas
  application
  platform_display
  platform_system
  platform_usb
)

target_include_directories(platform_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
