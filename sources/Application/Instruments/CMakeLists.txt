add_library(application_instruments
  CommandList.cpp
  Filters.cpp
  I_Instrument.cpp
  InstrumentBank.cpp
  InstrumentNameVariable.cpp
  MidiInstrument.cpp
  OpalInstrument.cpp
  NoneInstrument.cpp
  SRPUpdaters.cpp
  SampleInstrument.cpp
  SamplePool.cpp
  SampleVariable.cpp
  SIDInstrument.cpp
  SoundSource.cpp
  WavFile.cpp
  WavFileWriter.cpp
  WavHeader.cpp
)

target_link_libraries(application_instruments PUBLIC
  foundation_services
  system_io
  crsid
  opal
  SampleRate::samplerate
  etl
  application_utils
  profiler
)

target_include_directories(application_instruments PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
