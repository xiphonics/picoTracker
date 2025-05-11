add_library(application_mixer
  MixBus.cpp
  MixerService.cpp
)

target_link_libraries(application_mixer PUBLIC
  application_audio
)

target_include_directories(application_mixer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
