add_library(uiframework_interfaces
  I_GUIWindowImp.cpp
)

target_include_directories(uiframework_interfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
