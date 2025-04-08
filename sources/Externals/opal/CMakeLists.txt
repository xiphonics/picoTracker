add_library(opal
opal.h opal.cpp
)

# Disable the aggressive-loop-optimizations warning that can cause build errors with GCC14
target_compile_options(opal PRIVATE -Wno-aggressive-loop-optimizations)

target_link_libraries(opal 
    PUBLIC application_utils
    PUBLIC hardware_flash
)

target_include_directories(opal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
