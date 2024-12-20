add_library(platform_usb
  usb_utils.h usb_utils.cpp 
  usb_descriptors.c
)
target_link_libraries(platform_usb 
  PRIVATE pico_stdlib 
  PUBLIC tinyusb_device
  PUBLIC pico_unique_id
)

target_include_directories(platform_usb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  
include_directories(${PROJECT_SOURCE_DIR})