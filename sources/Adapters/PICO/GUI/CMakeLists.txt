add_library(hw_gui
  GUIFactory.h GUIFactory.cpp
  HWGUIWindowImp.h HWGUIWindowImp.cpp
  HWEventManager.h HWEventManager.cpp
)

target_link_libraries(hw_gui PUBLIC uiframework_simplebaseclasses
                             PUBLIC system_console
                             PUBLIC uiframework_basicdatas
                             PUBLIC application
                             PUBLIC ili9341
                             PUBLIC pico_system
)

target_include_directories(hw_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
