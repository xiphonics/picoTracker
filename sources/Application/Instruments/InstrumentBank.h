#ifndef _INSTRUMENT_BANK_H_
#define _INSTRUMENT_BANK_H_

#include "Application/Instruments/I_Instrument.h"
#include "Application/Model/Song.h"
#include "Application/Persistency/Persistent.h"
#include "MidiInstrument.h"
#include "SampleInstrument.h"
#include "TinysynthInstrument.h"

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
  unsigned short GetNext();
  unsigned short Clone(unsigned short i);

private:
  I_Instrument *instrument_[MAX_INSTRUMENT_COUNT];
  SampleInstrument si0;
  SampleInstrument si1;
  SampleInstrument si2;
  SampleInstrument si3;
  SampleInstrument si4;
  SampleInstrument si5;
  SampleInstrument si6;
  SampleInstrument si7;
  SampleInstrument si8;
  SampleInstrument si9;
  SampleInstrument si10;
  SampleInstrument si11;
  SampleInstrument si12;
  SampleInstrument si13;
  SampleInstrument si14;
  SampleInstrument si15;
  MidiInstrument mi0;
  MidiInstrument mi1;
  MidiInstrument mi2;
  MidiInstrument mi3;
  MidiInstrument mi4;
  MidiInstrument mi5;
  MidiInstrument mi6;
  MidiInstrument mi7;
  MidiInstrument mi8;
  MidiInstrument mi9;
  MidiInstrument mi10;
  MidiInstrument mi11;
  MidiInstrument mi12;
  MidiInstrument mi13;
  MidiInstrument mi14;
  MidiInstrument mi15;
  TinysynthInstrument tsi0;
};

#endif
