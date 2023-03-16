add_executable(lgpt
  PICOMain.cpp
)

target_link_libraries(lgpt PUBLIC pico_stdlib
                           PUBLIC pico_system
                           PUBLIC platform
)

target_include_directories(lgpt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(lgpt)
