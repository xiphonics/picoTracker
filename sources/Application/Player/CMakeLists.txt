add_library(application_player
  Player.h Player.cpp
  PlayerChannel.h PlayerChannel.cpp
  PlayerMixer.h PlayerMixer.cpp
  SyncMaster.h SyncMaster.cpp
  TablePlayback.h TablePlayback.cpp
)

target_link_libraries(application_player PUBLIC application_mixer
                                         PUBLIC application_audio
                                         PUBLIC application_model
                                         PUBLIC sdfat
                                         PUBLIC platform_sdcard
)

target_include_directories(application_player PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
