add_library(uiframework_framework INTERFACE)
# TODO: Cleanup, doesn't appear to be used
#  GUIParentGraphics.h GUIParentGraphics.cpp

target_include_directories(uiframework_framework INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
