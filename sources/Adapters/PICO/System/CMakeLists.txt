add_library(pico_system
  PICOSystem.h PICOSystem.cpp
  PICOEventQueue.h PICOEventQueue.cpp
  input.h input.cpp
)

target_link_libraries(pico_system INTERFACE system_system
                                  PUBLIC uiframework_simplebaseclasses
                                  PUBLIC hw_timer
                                  PUBLIC hw_audio
                                  PUBLIC services_audio
                                  PUBLIC pico_midi
                                  PUBLIC pico_filesystem
                                  PUBLIC hw_process
                                  PUBLIC hw_gui
                                  PUBLIC ili9341
)

target_include_directories(pico_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
