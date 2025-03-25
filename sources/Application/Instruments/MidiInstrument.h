#ifndef _MIDI_INSTRUMENT_H_
#define _MIDI_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "Externals/etl/include/etl/string.h"
#include "I_Instrument.h"
#include "Services/Midi/MidiMessage.h"
#include "Services/Midi/MidiService.h"

#define MAX_MIDI_CHORD_NOTES 4

const int MAX_MIDI_INSTRUMENT_NAME_LENGTH = 12;

class MidiInstrument : public I_Instrument {

public:
  MidiInstrument();
  virtual ~MidiInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual InstrumentType GetType() { return IT_MIDI; };

  virtual etl::string<24> GetName();

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };
  // external parameter list

  void SetChannel(int i);

private:
  etl::list<Variable *, 6> variables_;

  etl::array<uint8_t, MAX_MIDI_CHORD_NOTES + 1> lastNotes_[SONG_CHANNEL_COUNT];
  int remainingTicks_;
  bool playing_;
  bool retrig_;
  int retrigLoop_;
  char velocity_ = 127;
  TableSaveState tableState_;
  bool first_[SONG_CHANNEL_COUNT];

  Variable channel_;
  Variable noteLen_;
  Variable volume_;
  Variable table_;
  Variable tableAuto_;
  Variable name_;

  static MidiService *svc_;
};

#endif
