
#ifndef _TINYSYNTH_INSTRUMENT_H_
#define _TINYSYNTH_INSTRUMENT_H_

#include "I_Instrument.h"

#include "Application/Model/Song.h"
#include "Externals/tinysynth/tinysynth.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

#define BIP_ATTACK MAKE_FOURCC('A', 'T', 'C', 'K')
#define BIP_DECAY MAKE_FOURCC('D', 'E', 'C', 'Y')
#define BIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define BIP_CRUSH MAKE_FOURCC('C', 'R', 'S', 'H')

class TinysynthInstrument : public I_Instrument, I_Observer {

public:
  TinysynthInstrument();
  virtual ~TinysynthInstrument();
};

#endif