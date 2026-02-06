add_executable(picoTracker
  picoTrackerMain.cpp
)

target_link_libraries(picoTracker PUBLIC

    # --- Platform-specific Adapter Libraries (Highest Level) ---
    # These depend on the System and Foundation interfaces
    platform_system
    platform_audio
    platform_gui
    platform_timer
    platform_mutex
    platform_display
    platform_usb
    platform_midi
    platform_utils
    platform_filesystem 

    # --- Application Libraries ---
    # These depend on Services and Foundation
    application_views
    application_views_baseclasses
    application_model

    # --- Service Libraries ---
    services_audio

    # --- System Libraries ---
    # These define interfaces that the adapters implement
    system_timer
    system_console
    profiler

    # --- Foundation Libraries (Lowest-level, most things depend on these) ---
    foundation_services
    foundation

    # --- External and SDK libraries ---
    sdfat_fatlib
    pico_sdk_bundle
)

target_include_directories(picoTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})


# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoTracker)
