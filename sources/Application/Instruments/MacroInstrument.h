#ifndef _MACRO_INSTRUMENT_H_
#define _MACRO_INSTRUMENT_H_

#include "I_Instrument.h"

#include "Application/Model/Song.h"
#include "Externals/braids/envelope.h"
#include "Externals/braids/macro_oscillator.h"
#include "Externals/braids/quantizer.h"
#include "Externals/braids/signature_waveshaper.h"
#include "Externals/braids/vco_jitter_source.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

#define BIP_SHAPE MAKE_FOURCC('S', 'H', 'P', 'E')
#define BIP_TIMBRE MAKE_FOURCC('T', 'I', 'M', 'B')
#define BIP_COLOR MAKE_FOURCC('C', 'O', 'L', 'R')
#define BIP_ATTACK MAKE_FOURCC('A', 'T', 'C', 'K')
#define BIP_DECAY MAKE_FOURCC('D', 'E', 'C', 'Y')
#define BIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define BIP_CRUSH MAKE_FOURCC('C', 'R', 'S', 'H')

class MacroInstrument : public I_Instrument, I_Observer {

public:
  MacroInstrument();
  virtual ~MacroInstrument();
  // I_Instrument implementation
  virtual bool Init();
  virtual bool Start(int channel, unsigned char note, bool trigger = true);
  virtual void Stop(int channel);
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual bool IsInitialized();
  virtual bool IsEmpty();

  virtual InstrumentType GetType() { return IT_MACRO; };
  virtual const char *GetName(); // returns sample name until real
                                 // namer is implemented
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual void Purge();
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

  // Engine playback  start callback
  virtual void OnStart();

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);

protected:
private:
  braids::MacroOscillator osc_;
  braids::Envelope envelope_;
  braids::Quantizer quantizer_;
  braids::SignatureWaveshaper ws_;
  braids::VcoJitterSource jitter_source_;

  braids::MacroOscillatorShape shape_;
  uint16_t gain_lp_;
  static uint8_t sync_samples_[4096];
};
#endif
