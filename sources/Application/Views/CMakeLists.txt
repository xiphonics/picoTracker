add_library(application_views
  ChainView.h ChainView.cpp
  ConsoleView.h ConsoleView.cpp
  GrooveView.h GrooveView.cpp
  InstrumentView.h InstrumentView.cpp
  ListSelectView.h ListSelectView.cpp
  MixerView.h MixerView.cpp
  NullView.h NullView.cpp
  PhraseView.h PhraseView.cpp
  ProjectView.h ProjectView.cpp
  SongView.h SongView.cpp
  TableView.h TableView.cpp
  UIController.h UIController.cpp
  ViewData.h ViewData.cpp
)

target_link_libraries(application_views PUBLIC application_views_baseclasses
                                        PUBLIC application_instruments
                                        PUBLIC hardware_watchdog
)

target_include_directories(application_views PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(BaseClasses)
add_subdirectory(ModalDialogs)
