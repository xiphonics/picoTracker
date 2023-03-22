#ifndef _CRSID_INSTRUMENT_H_
#define _CRSID_INSTRUMENT_H_

#include "Externals/cRSID/SID.h"
#include "I_Instrument.h"

#define DIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define DIP_TABLE MAKE_FOURCC('T', 'A', 'B', 'L')
#define DIP_TABLEAUTO MAKE_FOURCC('T', 'B', 'L', 'A')

static const unsigned short sid_notes[96] = {
    0x0112, 0x0123, 0x0134, 0x0146, 0x015A, 0x016E, 0x0184, 0x018B, 0x01B3,
    0x01CD, 0x01E9, 0x0206, 0x0225, 0x0245, 0x0268, 0x028C, 0x02B3, 0x02DC,
    0x0308, 0x0336, 0x0367, 0x039B, 0x03D2, 0x040C, 0x0449, 0x048B, 0x04D0,
    0x0519, 0x0567, 0x05B9, 0x0610, 0x066C, 0x06CE, 0x0735, 0x07A3, 0x0817,
    0x0893, 0x0915, 0x099F, 0x0A32, 0x0ACD, 0x0B72, 0x0C20, 0x0CD8, 0x0D9C,
    0x0E6B, 0x0F46, 0x102F, 0x1125, 0x122A, 0x133F, 0x1464, 0x159A, 0x16E3,
    0x183F, 0x1981, 0x1B38, 0x1CD6, 0x1E80, 0x205E, 0x224B, 0x2455, 0x267E,
    0x28C8, 0x2B34, 0x2DC6, 0x2DC6, 0x3361, 0x366F, 0x39AC, 0x3D1A, 0x40BC,
    0x4495, 0x48A9, 0x4CFC, 0x518F, 0x5669, 0x5B8C, 0x60FE, 0x6602, 0x6CDF,
    0x7358, 0x7A34, 0x8178, 0x892B, 0x9153, 0x99F7, 0xA31F, 0xACD2, 0xB719,
    0xC1FC, 0xCD85, 0xD9BD, 0xE6B0, 0xF467, 0xFFFF}; // last one is 0x1F2F0

class SIDInstrument
    : public I_Instrument {

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
