add_library(platform_system
  picoTrackerSystem.h picoTrackerSystem.cpp
  picoTrackerEventQueue.h picoTrackerEventQueue.cpp
  input.h input.cpp
)

target_link_libraries(platform_system INTERFACE system_system
                                      PUBLIC uiframework_simplebaseclasses
                                      PUBLIC platform_timer
                                      PUBLIC platform_audio
                                      PUBLIC services_audio
                                      PUBLIC platform_midi
                                      PUBLIC platform_gui
                                      PUBLIC platform_display
                                      PUBLIC dummy_midi
                                      PUBLIC pico_stdlib
                                      PUBLIC hardware_gpio
                                      PUBLIC hardware_adc
                                      PUBLIC etl
)


target_include_directories(platform_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
