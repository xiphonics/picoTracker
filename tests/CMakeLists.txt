cmake_minimum_required(VERSION 3.13)

project(picoTrackerHostTests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(picoTracker_tests
  test_main.cpp
  foundation_types_tests.cpp
  wavheader_tests.cpp
  ../sources/Application/Instruments/WavHeader.cpp
)

target_include_directories(picoTracker_tests PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/stubs
  ${CMAKE_CURRENT_LIST_DIR}/../sources/Externals
  ${CMAKE_CURRENT_LIST_DIR}/../sources/Externals/SRC/include
  ${CMAKE_CURRENT_LIST_DIR}/../sources
)

target_compile_definitions(picoTracker_tests PRIVATE
  HOST_TEST
)
