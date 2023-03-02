add_library(hw_process
  HWProcess.h HWProcess.cpp
  )

target_link_libraries(hw_process PUBLIC system_process
                                 PUBLIC pico_stdlib
#                                 PUBLIC freertos
)

target_include_directories(hw_process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
