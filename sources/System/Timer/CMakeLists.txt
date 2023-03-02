add_library(system_timer
  Timer.h Timer.cpp
)

target_include_directories(system_timer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
