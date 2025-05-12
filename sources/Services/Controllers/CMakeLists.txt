add_library(services_controllers
  ButtonControllerSource.cpp
  Channel.cpp
  ControlNode.cpp
  ControllableVariable.cpp
  ControllerService.cpp
  ControllerSource.cpp
  HatControllerSource.cpp
  JoystickControllerSource.cpp
  KeyboardControllerSource.cpp
  MultiChannelAdapter.cpp
)

target_include_directories(services_controllers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
