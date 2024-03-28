add_library(application_instruments
  CommandList.h CommandList.cpp
  Filters.h Filters.cpp
  I_Instrument.h
  I_SRPUpdater.h
  InstrumentBank.h InstrumentBank.cpp
  MidiInstrument.h MidiInstrument.cpp
  SRPUpdaters.h SRPUpdaters.cpp
  SampleInstrument.h SampleInstrument.cpp
  SampleInstrumentDatas.h
  SamplePool.h SamplePool.cpp
  SampleRenderingParams.h
  SampleVariable.h SampleVariable.cpp
  SoundSource.h SoundSource.cpp
  TinysynthInstrument.h TinysynthInstrument.cpp
  WavFile.h WavFile.cpp
  WavFileWriter.h WavFileWriter.cpp
)

target_link_libraries(application_instruments PUBLIC foundation_services
                                              PUBLIC system_io
#                                              PUBLIC soundfont
                                              PUBLIC pico_stdlib
                                              PUBLIC hardware_flash
                                              PUBLIC pico_stdlib                            
                                              PUBLIC hardware_flash
                                              PUBLIC tinysynth
)

target_include_directories(application_instruments PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
