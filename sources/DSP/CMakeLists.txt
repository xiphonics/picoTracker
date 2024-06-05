set(project_name DSP)

add_executable(${project_name}
        main.cpp
        )

# pull in common dependencies
target_link_libraries(${project_name} PRIVATE pico_stdlib
  )

# create map/bin/hex file etc.
pico_add_extra_outputs(${project_name})

pico_enable_stdio_usb(${project_name} 0)
pico_enable_stdio_uart(${project_name} 1)
pico_set_binary_type(${project_name} no_flash)

set_property(TARGET ${project_name} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")

add_custom_command(TARGET ${project_name}
  POST_BUILD
  COMMAND xxd -n ${project_name} -i ${PROJECT_BINARY_DIR}/${project_name}/${project_name}.elf | sed  -e '1s/^/const /' > ${PROJECT_SOURCE_DIR}/Adapters/picoTracker/platform/DSP.h
)
