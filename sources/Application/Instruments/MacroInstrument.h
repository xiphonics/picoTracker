#ifndef _MACRO_INSTRUMENT_H_
#define _MACRO_INSTRUMENT_H_

#include "I_Instrument.h"
#include "pico/stdlib.h"

#include "Application/Model/Song.h"
#include "Externals/braids/envelope.h"
#include "Externals/braids/macro_oscillator.h"
#include "Externals/braids/quantizer.h"
#include "Externals/braids/signature_waveshaper.h"
#include "Externals/braids/vco_jitter_source.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

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

  // TODO: set a real instrument type before shipping macro instruments
  virtual InstrumentType GetType() { return IT_NONE; };
  virtual etl::string<24> GetName(); // returns sample name until real
                                     // namer is implemented
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual void Purge();
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  // Engine playback  start callback
  virtual void OnStart();

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);

protected:
private:
  etl::list<Variable *, 6> variables_;

  bool running_;

  braids::MacroOscillator osc_;
  braids::Envelope envelope_;
  braids::Quantizer quantizer_;
  braids::SignatureWaveshaper ws_;
  braids::VcoJitterSource jitter_source_;

  braids::MacroOscillatorShape osc_shape_;

  Variable shape_;
  Variable timbre_;
  Variable color_;
  Variable attack_;
  Variable decay_;
  Variable signature_;

  uint16_t gain_lp_;
  uint16_t remain_;
};
#endif
