add_library(application_model
  Chain.h Chain.cpp
  Config.h Config.cpp
  Groove.h Groove.cpp
  Mixer.h Mixer.cpp
  Phrase.h Phrase.cpp
  Project.h Project.cpp
  Scale.h Scale.cpp
  Song.h Song.cpp
  Table.h Table.cpp
  Theme.h Theme.cpp
  )

target_link_libraries(application_model PUBLIC pico_stdlib
                                        PUBLIC system_system
                                        PUBLIC foundation_variables
                                        PUBLIC sdfat
                                        PUBLIC platform_sdcard
)

target_include_directories(application_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
