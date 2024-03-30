
#ifndef _TINYSYNTH_INSTRUMENT_H_
#define _TINYSYNTH_INSTRUMENT_H_

#include "I_Instrument.h"

#include "Application/Model/Song.h"
#include "Application/Utils/fixed.h"
#include "Externals/tinysynth/tinysynth.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

#define TSIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define TXIP_H1 MAKE_FOURCC('T', 'X', 'H', '1')

class TinysynthInstrument : public I_Instrument, I_Observer {

public:
  TinysynthInstrument();
  virtual ~TinysynthInstrument();

  // I_Instrument implementation
  virtual bool Init();
  virtual bool Start(int channel, unsigned char note, bool trigger = true);
  virtual void Stop(int channel);
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual bool IsInitialized();
  virtual bool IsEmpty();

  // Engine playback  start callback
  virtual void OnStart();

  virtual etl::string<24> GetName();

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);

  virtual InstrumentType GetType() { return IT_TINYSYNTH; };
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual void Purge();
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

private:
  bool running_;
  TinySynth *tinysynth_;
  Variable *volume_;
};

#endif