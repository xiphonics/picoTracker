add_library(application_views_modaldialogs
  MessageBox.cpp
  TextInputModalView.cpp
  RenderProgressModal.cpp
)

target_link_libraries(application_views_modaldialogs PUBLIC
  application_views_baseclasses
  system_filesystem
)

target_include_directories(application_views_modaldialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
