add_library(platform_usb
  usb_utils.cpp 
  usb_descriptors.c
)
target_link_libraries(platform_usb PUBLIC
  tinyusb_device
)

target_include_directories(platform_usb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  
include_directories(${PROJECT_SOURCE_DIR})
