#ifndef _MIDI_INSTRUMENT_H_
#define _MIDI_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "I_Instrument.h"
#include "Services/Midi/MidiService.h"

#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CC 0xB0
#define MIDI_PRG 0xC0

#define MIP_CHANNEL MAKE_FOURCC('C', 'H', 'N', 'L')
#define MIP_NOTELENGTH MAKE_FOURCC('L', 'E', 'N', 'G')
#define MIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define MIP_PAN MAKE_FOURCC('P', 'A', 'N', ' ')
#define MIP_TABLE MAKE_FOURCC('T', 'A', 'B', 'L')
#define MIP_TABLEAUTO MAKE_FOURCC('T', 'B', 'L', 'A')

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

  // external parameter list

  void SetChannel(int i);

private:
  uint8_t lastNote_[SONG_CHANNEL_COUNT];
  int remainingTicks_;
  bool playing_;
  bool retrig_;
  bool porto_;
  char porto_pending_[SONG_CHANNEL_COUNT];
  int retrigLoop_;
  char velocity_ = 127;
  TableSaveState tableState_;
  bool first_[SONG_CHANNEL_COUNT];

  static MidiService *svc_;
};

#endif
