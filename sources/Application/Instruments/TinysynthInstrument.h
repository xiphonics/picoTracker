
#ifndef _TINYSYNTH_INSTRUMENT_H_
#define _TINYSYNTH_INSTRUMENT_H_

#include "I_Instrument.h"

#include "Application/Model/Song.h"
#include "Externals/tinysynth/tinysynth.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

#define TSIP_VOL1 MAKE_FOURCC('T', 'S', 'V', '1')

class TinysynthInstrument : public I_Instrument, I_Observer {

public:
  TinysynthInstrument();
  virtual ~TinysynthInstrument();

private:
  TinySynth tinysynth_;
  bool playing_;
};

#endif