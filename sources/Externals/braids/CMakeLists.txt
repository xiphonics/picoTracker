add_library(braids
  macro_oscillator.h macro_oscillator.cc
  analog_oscillator.h analog_oscillator.cc
  digital_oscillator.h digital_oscillator.cc
  resources.h resources.cc
  settings.h settings.cc
  excitation.h
  svf.h
  parameter_interpolation.h
  dsp.h
  macros.h
  random.h random.cc
  envelope.h
  quantizer.h quantizer.cc
  signature_waveshaper.h
  vco_jitter_source.h
)

target_link_libraries(braids PUBLIC pico_stdlib
)

target_include_directories(braids PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
