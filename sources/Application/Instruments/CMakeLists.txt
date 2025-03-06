add_library(application_instruments
  CommandList.h CommandList.cpp
  Filters.h Filters.cpp
  I_Instrument.h
  I_SRPUpdater.h
  InstrumentBank.h InstrumentBank.cpp
  MacroInstrument.h MacroInstrument.cpp
  MidiInstrument.h MidiInstrument.cpp
  OpalInstrument.h OpalInstrument.cpp
  NoneInstrument.h NoneInstrument.cpp
  SRPUpdaters.h SRPUpdaters.cpp
  SampleInstrument.h SampleInstrument.cpp
  SampleInstrumentDatas.h
  SamplePool.h SamplePool.cpp
  SampleRenderingParams.h
  SampleVariable.h SampleVariable.cpp
  SIDInstrument.h SIDInstrument.cpp
  SoundSource.h SoundSource.cpp
  WavFile.h WavFile.cpp
  WavFileWriter.h WavFileWriter.cpp
)

target_link_libraries(application_instruments PUBLIC foundation_services
                                              PUBLIC system_io
                                              PUBLIC pico_stdlib
                                              PUBLIC hardware_flash
                                              PUBLIC pico_stdlib                            
                                              PUBLIC hardware_flash
                                              PUBLIC crsid
                                              PUBLIC opal
                                              PUBLIC braids
                                              PUBLIC pico_rand
                                              PUBLIC sdfat
                                              PUBLIC platform_sdcard
                                              PUBLIC services_time
                                              PUBLIC etl
                                              PUBLIC application_utils
)

target_include_directories(application_instruments PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
