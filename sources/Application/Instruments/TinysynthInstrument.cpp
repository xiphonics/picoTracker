#include "TinysynthInstrument.h"

TinysynthInstrument::TinysynthInstrument() {

  Variable *v = new Variable("volume1", TSIP_VOL1, 0x7f);
  insert(end(), v);
}

TinysynthInstrument::~TinysynthInstrument() {}