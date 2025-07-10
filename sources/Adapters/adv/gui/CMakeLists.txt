add_library(platform_gui
  GUIFactory.cpp
  advGUIWindowImp.cpp
  advEventManager.cpp
  SerialDebugUI.h SerialDebugUI.cpp
#  picoRemoteUI.h picoRemoteUI.cpp
)

target_link_libraries(platform_gui PUBLIC uiframework_simplebaseclasses
                                   PUBLIC system_console
                                   PUBLIC uiframework_basicdatas
                                   PUBLIC application
                                   PUBLIC platform_display
                                   PUBLIC platform_system
                                   PRIVATE platform_utils
                                   tinyusb
)

target_include_directories(platform_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
