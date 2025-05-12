add_library(opal
  opal.cpp
)

# Disable the aggressive-loop-optimizations warning that can cause build errors with GCC14
# this is causeed only by the code changes in opal.cpp lines 241-247 due to code for 4-Op
# which would cause out of bounds access but doesnt because 4-Op are currently disabled
target_compile_options(opal PRIVATE -Wno-aggressive-loop-optimizations)

target_link_libraries(opal 
    PUBLIC application_utils
)

target_include_directories(opal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
