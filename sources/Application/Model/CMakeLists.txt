add_library(application_model
  Chain.h Chain.cpp
  Config.h Config.cpp
  Groove.h Groove.cpp
  Mixer.h Mixer.cpp
  Phrase.h Phrase.cpp
  Project.h Project.cpp
  Song.h Song.cpp
  Table.h Table.cpp
  )

target_link_libraries(application_model PUBLIC system_system
                                        PUBLIC foundation_variables
                                        PUBLIC tiny_xml
)

target_include_directories(application_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: check this, how to do properly? this finds System/System/System.h
# include_directories(/Users/fran/github/LittleGPTracker-pico/sources/)
include_directories(${PROJECT_SOURCE_DIR})
