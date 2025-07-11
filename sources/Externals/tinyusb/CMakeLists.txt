add_library(tinyusb
  # common
  src/tusb.c
  src/common/tusb_fifo.c
  # device
  src/device/usbd.c
  src/device/usbd_control.c
  src/class/audio/audio_device.c
  src/class/cdc/cdc_device.c
  src/class/dfu/dfu_device.c
  src/class/dfu/dfu_rt_device.c
  src/class/hid/hid_device.c
  src/class/midi/midi_device.c
  src/class/msc/msc_device.c
  src/class/net/ecm_rndis_device.c
  src/class/net/ncm_device.c
  src/class/usbtmc/usbtmc_device.c
  src/class/vendor/vendor_device.c
  src/class/video/video_device.c
  # host
  src/host/usbh.c
  src/host/hub.c
  src/class/cdc/cdc_host.c
  src/class/hid/hid_host.c
  src/class/midi/midi_host.c
  src/class/msc/msc_host.c
  src/class/vendor/vendor_host.c
  # typec
  src/typec/usbc.c
  # driver
  src/portable/synopsys/dwc2/dcd_dwc2.c
  src/portable/synopsys/dwc2/dwc2_common.c
  src/portable/synopsys/dwc2/hcd_dwc2.c
)

target_link_libraries(tinyusb PUBLIC
  stm32cubemx
  freertos_kernel
)

target_include_directories(tinyusb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_SOURCE_DIR})
