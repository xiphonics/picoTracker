add_library(uiframework_simplebaseclasses
  EventManager.cpp
  GUIWindow.cpp
)

target_link_libraries(uiframework_simplebaseclasses PUBLIC
  application_model
)

target_include_directories(uiframework_simplebaseclasses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
