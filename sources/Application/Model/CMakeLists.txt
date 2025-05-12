add_library(application_model
  Chain.cpp
  Config.cpp
  Groove.cpp
  Mixer.cpp
  Phrase.cpp
  Project.cpp
  Scale.cpp
  Song.cpp
  Table.cpp
)

target_link_libraries(application_model PUBLIC
  system_system
  foundation_variables
  application_utils
)

target_include_directories(application_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
