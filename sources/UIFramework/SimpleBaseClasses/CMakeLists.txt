add_library(uiframework_simplebaseclasses
  EventManager.h EventManager.cpp
  GUIWindow.h GUIWindow.cpp
)

target_link_libraries(pico_system PUBLIC application_model
                                  PUBLIC application_controllers
)

target_include_directories(uiframework_simplebaseclasses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
