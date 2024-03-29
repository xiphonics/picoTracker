
#ifndef _TINYSYNTH_INSTRUMENT_H_
#define _TINYSYNTH_INSTRUMENT_H_

#include "I_Instrument.h"

#include "Application/Model/Song.h"
#include "Application/Utils/fixed.h"
#include "Externals/tinysynth/tinysynth.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

#define TSIP_VOL1 MAKE_FOURCC('T', 'S', 'V', '1')

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

  virtual etl::string<24> GetName(); // returns sample name until real
                                     // name is implemented

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);

  virtual InstrumentType GetType() { return IT_SAMPLE; };
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual void Purge();
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

private:
  TinySynth *tinysynth_;
};

#endif