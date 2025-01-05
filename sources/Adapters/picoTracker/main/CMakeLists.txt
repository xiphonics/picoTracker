add_executable(picoTracker
  picoTrackerMain.cpp
)

target_link_libraries(picoTracker PUBLIC pico_stdlib
                                  PUBLIC platform_system
                                  PUBLIC platform
                                  PUBLIC tinyusb_board
                                  PUBLIC tinyusb_device
                                  PUBLIC platform_usb
)

target_include_directories(picoTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})


# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoTracker)
