add_library(application
  AppWindow.h AppWindow.cpp
  Application.h Application.cpp
)

target_link_libraries(application PUBLIC uiframework_simplebaseclasses
                                  PUBLIC application_mixer
                                  PUBLIC system_process
                                  PUBLIC application_commands
                                  PUBLIC services_midi
                                  PUBLIC application_controllers
                                  PUBLIC application_instruments
                                  PUBLIC application_model
                                  PUBLIC application_persistency
                                  PUBLIC application_player
                                  PUBLIC application_views_baseclasses
                                  PUBLIC application_views
                                  PUBLIC application_views_modaldialogs
                                  PUBLIC uiframework_basicdatas
                                  PUBLIC foundation_variables
)

target_include_directories(application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(Model)
add_subdirectory(Instruments)
add_subdirectory(Player)
add_subdirectory(Mixer)
add_subdirectory(Utils)
add_subdirectory(Controllers)
add_subdirectory(Commands)
add_subdirectory(Persistency)
add_subdirectory(Views)
add_subdirectory(Audio)
