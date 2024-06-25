add_library(application_views_modaldialogs
  # PicoImportSampleDialog.h PicoImportSampleDialog.cpp
  MessageBox.h MessageBox.cpp
  NewProjectDialog.h NewProjectDialog.cpp
  SelectProjectDialog.h SelectProjectDialog.cpp
)

target_link_libraries(application_views_modaldialogs PUBLIC application_views_baseclasses
                                                     PUBLIC pico_multicore
                                                     PUBLIC system_filesystem
                                                     PUBLIC sdfat
                                                     PUBLIC platform_sdcard
  
)

target_include_directories(application_views_modaldialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
