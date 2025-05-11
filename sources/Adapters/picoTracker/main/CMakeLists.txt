add_executable(picoTracker
  picoTrackerMain.cpp
)

target_link_libraries(picoTracker PUBLIC
  platform_system
  platform
  tinyusb_board
  tinyusb_device
  platform_usb
  platform_filesystem
)

target_include_directories(picoTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})


# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoTracker)
