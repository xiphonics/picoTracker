cmake_minimum_required(VERSION 3.13)

project(picoTrackerHostTests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(picoTracker_tests
  test_main.cpp
  allocation_tests.cpp
  ../sources/Application/Model/Chain.cpp
  ../sources/Application/Model/Phrase.cpp
  foundation_types_tests.cpp
  delayed_rows_tests.cpp
  row_fx_tests.cpp
  ../sources/Application/Instruments/CommandList.cpp
  ../sources/Application/Player/RowFx.cpp
  filesystem_path_tests.cpp
  samplecache_tests.cpp
  wavheader_tests.cpp
  ../sources/Application/Instruments/WavHeader.cpp
  ../sources/Application/Instruments/SampleCacheValidate.cpp
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

add_executable(midi_input_tests
  test_main.cpp
  midi_input_tests.cpp
  ../sources/Foundation/Observable.cpp
  ../sources/Services/Midi/MidiInDevice.cpp
  ../sources/Services/Midi/MidiNoteTracker.cpp
  ../sources/Adapters/picoTracker/midi/picoTrackerUSBMidiInDevice.cpp
)
target_include_directories(midi_input_tests PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/midi_stubs
  ${CMAKE_CURRENT_LIST_DIR}/stubs
  ${CMAKE_CURRENT_LIST_DIR}/../sources/Externals
  ${CMAKE_CURRENT_LIST_DIR}/../sources
)
target_compile_definitions(midi_input_tests PRIVATE HOST_TEST)

add_executable(midi_instrument_tests
  test_main.cpp
  midi_instrument_tests.cpp
  midi_instrument_link_stubs.cpp
  ../sources/Application/Instruments/MidiInstrument.cpp
  ../sources/Application/Player/PlayerChannel.cpp
  ../sources/Application/Model/Scale.cpp
  ../sources/Foundation/Observable.cpp
  ../sources/Foundation/Variables/Variable.cpp
  ../sources/Foundation/Variables/VariableContainer.cpp
)
target_include_directories(midi_instrument_tests PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/instrument_stubs
  ${CMAKE_CURRENT_LIST_DIR}/stubs
  ${CMAKE_CURRENT_LIST_DIR}/../sources/Externals
  ${CMAKE_CURRENT_LIST_DIR}/../sources
)
target_compile_definitions(midi_instrument_tests PRIVATE HOST_TEST)
