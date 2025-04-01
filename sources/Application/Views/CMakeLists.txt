add_library(application_views
  FieldView.h FieldView.cpp
  ChainView.h ChainView.cpp
  ConsoleView.h ConsoleView.cpp
  GrooveView.h GrooveView.cpp
  ImportView.h ImportView.cpp
  InstrumentImportView.h InstrumentImportView.cpp
  InstrumentView.h InstrumentView.cpp
  DeviceView.h DeviceView.cpp
  NullView.h NullView.cpp
  PhraseView.h PhraseView.cpp
  ProjectView.h ProjectView.cpp
  SelectProjectView.h SelectProjectView.cpp
  SongView.h SongView.cpp
  TableView.h TableView.cpp
  UIController.h UIController.cpp
  ViewData.h ViewData.cpp
  ScreenView.h ScreenView.cpp
  MixerView.h MixerView.cpp
)

target_link_libraries(application_views PUBLIC application_views_baseclasses
                                        PUBLIC application_instruments
                                        PUBLIC hardware_watchdog
                                        PUBLIC etl
                                        PUBLIC sdfat
                                        PUBLIC platform_sdcard
                                        PUBLIC pico_multicore
                                        PUBLIC application_utils
)

target_include_directories(application_views PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(BaseClasses)
add_subdirectory(ModalDialogs)
