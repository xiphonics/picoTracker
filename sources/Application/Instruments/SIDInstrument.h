#ifndef _CRSID_INSTRUMENT_H_
#define _CRSID_INSTRUMENT_H_

#include "Externals/cRSID/SID.h"
#include "I_Instrument.h"

enum SIDInstrumentInstance { SID1 = 1, SID2 };

enum SIDInstrumentWaveform {
  DWF_NONE = 0,
  DWF_TRI,
  DWF_SAW,
  DWF_TRI_SAW,
  DWF_SQ,
  DWF_TRI_SQ,
  DWF_SAW_SQ,
  DWF_TRI_SAW_SQ,
  DWF_NOISE,
  DWF_LAST
};
enum SIDInstrumentFilterMode { DFM_LP = 0, DFM_BP, DFM_HP, DFM_LAST };

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

class SIDInstrument : public I_Instrument {

public:
  SIDInstrument(SIDInstrumentInstance chip);
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

  virtual InstrumentType GetType() { return IT_SID; };

  virtual etl::string<24> GetName();

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  SIDInstrumentInstance GetChip() { return chip_; };
  unsigned short GetOsc() { return osc_; };
  void SetRender(bool render) { render_ = render; };

private:
  etl::list<Variable *, 16> variables_;

  etl::string<24> name_;
  SIDInstrumentInstance chip_; // SID1 or SID2
  unsigned short osc_ = 0;     // 0, 1 or 2
  bool render_ = false;

  bool playing_;
  bool gate_;
  //  bool retrig_;
  // int retrigLoop_;
  TableSaveState tableState_;
  //  bool first_[SONG_CHANNEL_COUNT];
  //  reSID::SID sid_;
  //  reSID::cycle_count delta_t_;
  static cRSID sid1_;
  static cRSID sid2_;
  cRSID *sid_;
  static SIDInstrument *SID1RenderMaster;
  static SIDInstrument *SID2RenderMaster;
  //  static bool rendered1_;

  Variable vpw_;
  static Variable vwf1_;
  static Variable vwf2_;
  Variable *vwf_;
  Variable vsync_;
  Variable vring_;
  Variable vadsr_;
  Variable vfon_;

  static Variable fltcut1_;
  static Variable fltcut2_;
  Variable *fltcut_;
  static Variable fltres1_;
  static Variable fltres2_;
  Variable *fltres_;
  static Variable fltmode1_;
  static Variable fltmode2_;
  Variable *fltmode_;
  static Variable vol1_;
  static Variable vol2_;
  Variable *vol_;
};

#endif
