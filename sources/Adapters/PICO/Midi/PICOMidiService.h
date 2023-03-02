#ifndef _PICO_MIDI_SERVICE_H_
#define _PICO_MIDI_SERVICE_H_

#include "Services/Midi/MidiService.h"

class PICOMidiService : public MidiService {
public:
  PICOMidiService();
  ~PICOMidiService();

protected:
  virtual void buildDriverList();
};

#endif
