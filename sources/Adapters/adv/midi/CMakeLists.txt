add_library(platform_midi
  advMidiService.cpp
  advMidiOutDevice.cpp
  advUSBMidiOutDevice.cpp
)

target_link_libraries(platform_midi PRIVATE
  platform_usb
)

target_include_directories(platform_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

