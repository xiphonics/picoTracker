add_library(application_commands
  ApplicationCommandDispatcher.h ApplicationCommandDispatcher.cpp
  EventDispatcher.h EventDispatcher.cpp
  NodeList.h
)

target_link_libraries(application_commands PUBLIC system_process
                                        PUBLIC sdfat
                                        PUBLIC platform_sdcard
)

target_include_directories(application_commands PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
