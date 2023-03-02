add_library(hw_timer
  Timer.h Timer.cpp
)

target_link_libraries(hw_timer PUBLIC pico_stdlib
                               PUBLIC system_timer
                               PUBLIC foundation
)

target_include_directories(hw_timer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${PROJECT_SOURCE_DIR}
)

