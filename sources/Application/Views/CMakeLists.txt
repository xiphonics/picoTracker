add_library(application_views
  FieldView.cpp
  ChainView.cpp
  ConsoleView.cpp
  GrooveView.cpp
  ImportView.cpp
  InstrumentImportView.cpp
  InstrumentView.cpp
  ThemeImportView.cpp
  DeviceView.cpp
  NullView.cpp
  PhraseView.cpp
  ProjectView.cpp
  RecordView.cpp
  SelectProjectView.cpp
  SongView.cpp
  TableView.cpp
  ThemeView.cpp
  ToastView.cpp
  UIController.cpp
  ViewData.cpp
  ScreenView.cpp
  MixerView.cpp
  SampleEditorView.cpp
  SampleEditProgressDisplay.cpp
  SampleSlicesView.cpp
)

target_link_libraries(application_views PUBLIC
  application_views_baseclasses
  application_instruments
  etl
  application_utils
  platform_display
)

# only need to add platform_audio for pico builds
if(NOT ADV)
  target_link_libraries(application_views PUBLIC
    platform_audio
  )
endif()

target_include_directories(application_views PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(BaseClasses)
add_subdirectory(ModalDialogs)
