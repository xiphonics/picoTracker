# In sdk_bundle/CMakeLists.txt

# Define a new static library that will contain all the compiled SDK code.
add_library(pico_sdk_bundle STATIC "")

# List all the SDK interface libraries we need.
set(PICO_COMPONENTS
    pico_stdlib
    pico_sync
    pico_multicore
    pico_unique_id
    pico_multicore
    pico_rand
    platform_mutex
    hardware_spi
    hardware_pio
    hardware_dma
    hardware_flash
    hardware_pwm
    hardware_uart
    hardware_pll
    hardware_adc
    tinyusb_device
    tinyusb_board
    # hardware_watchdog
    # hardware_vreg
    )

# Link against the components to pull in their sources and usage requirements.
target_link_libraries(pico_sdk_bundle PUBLIC ${PICO_COMPONENTS})

# --- DELETE OR COMMENT OUT THE LINE BELOW ---
# pico_add_extra_outputs(pico_sdk_bundle)