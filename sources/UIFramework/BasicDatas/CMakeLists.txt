add_library(uiframework_basicdatas
  GUIEvent.cpp
  GUIRect.cpp
)

target_link_libraries(uiframework_basicdatas PUBLIC
  uiframework_interfaces
)

target_include_directories(uiframework_basicdatas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
