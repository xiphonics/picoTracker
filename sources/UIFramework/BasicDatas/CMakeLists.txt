add_library(uiframework_basicdatas
# TODO: Cleanup, these don't seem to be used
#  GUIBitmap.h GUIBitmap.cpp
  GUICreateWindowParams.h
  GUIEvent.h GUIEvent.cpp
  GUIPoint.h
  GUIRect.h GUIRect.cpp
)

target_link_libraries(uiframework_basicdatas PUBLIC uiframework_interfaces)

target_include_directories(uiframework_basicdatas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
