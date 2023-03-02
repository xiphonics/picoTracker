add_library(services_controllers
  ButtonControllerSource.h ButtonControllerSource.cpp
  Channel.h Channel.cpp
  ControlNode.h ControlNode.cpp
  ControllableVariable.h ControllableVariable.cpp
  ControllerService.h ControllerService.cpp
  ControllerSource.h ControllerSource.cpp
  HatControllerSource.h HatControllerSource.cpp
  JoystickControllerSource.h JoystickControllerSource.cpp
  KeyboardControllerSource.h KeyboardControllerSource.cpp
  MultiChannelAdapter.h MultiChannelAdapter.cpp
)

target_include_directories(services_controllers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
