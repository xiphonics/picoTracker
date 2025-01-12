#ifndef _INSTRUMENT_BANK_H_
#define _INSTRUMENT_BANK_H_

#include "Application/Instruments/I_Instrument.h"
#include "Application/Model/Song.h"
#include "Application/Persistency/Persistent.h"
#include "Externals/etl/include/etl/pool.h"
#include "MacroInstrument.h"
#include "MidiInstrument.h"
#include "NoneInstrument.h"
#include "OpalInstrument.h"
#include "SIDInstrument.h"
#include "SampleInstrument.h"

#define NO_MORE_INSTRUMENT 0x100

class InstrumentBank : public Persistent {
public:
  InstrumentBank();
  ~InstrumentBank();
  void AssignDefaults();
  I_Instrument *GetInstrument(int i);
  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);
  void Init();
  void OnStart();
  unsigned short GetNextAndAssignID(InstrumentType type, unsigned char id);
  void releaseInstrument(unsigned short id);
  unsigned short Clone(unsigned short i);

private:
  etl::array<I_Instrument *, MAX_INSTRUMENT_COUNT> instruments_;
  etl::pool<SampleInstrument, MAX_SAMPLEINSTRUMENT_COUNT> sampleInstrumentPool_;
  etl::pool<MidiInstrument, MAX_MIDIINSTRUMENT_COUNT> midiInstrumentPool_;
  etl::pool<SIDInstrument, MAX_SIDINSTRUMENT_COUNT> sidInstrumentPool_;
  etl::pool<OpalInstrument, MAX_OPALINSTRUMENT_COUNT> opalInstrumentPool_;
  etl::pool<MacroInstrument, MAX_MACROINSTRUMENT_COUNT> macroInstrumentPool_;
  NoneInstrument none_ = NoneInstrument();
  unsigned short sidOscCount = 0;
};

#endif
