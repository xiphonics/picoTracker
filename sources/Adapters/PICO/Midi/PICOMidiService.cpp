
#include "PICOMidiService.h"
#include "PICOSerialMidiDevice.h"

PICOMidiService::PICOMidiService(){};

PICOMidiService::~PICOMidiService(){};

void PICOMidiService::buildDriverList() { // Here we just loop over existing
                                          // Midi out and create a midi device
                                          // for each of them.

  Insert(new PICOSerialMidiOutDevice("PICO_UART1", uart1));
};
