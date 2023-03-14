#ifndef _CRSID_INSTRUMENT_H_
#define _CRSID_INSTRUMENT_H_

#include "Externals/cRSID/SID.h"
#include "I_Instrument.h"

#define DIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define DIP_TABLE MAKE_FOURCC('T', 'A', 'B', 'L')
#define DIP_TABLEAUTO MAKE_FOURCC('T', 'B', 'L', 'A')

class SIDInstrument : public I_Instrument {

public:
  SIDInstrument();
  virtual ~SIDInstrument();

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

  virtual const char *GetName();

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

private:
  char name_[20]; // Instrument name
  //  int lastNote_[SONG_CHANNEL_COUNT];
  //  int remainingTicks_;
  bool playing_;
  //  bool retrig_;
  // int retrigLoop_;
  TableSaveState tableState_;
  //  bool first_[SONG_CHANNEL_COUNT];
  //  reSID::SID sid_;
  //  reSID::cycle_count delta_t_;
  cRSID *sid_;
};

#endif
