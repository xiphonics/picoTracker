
#include "CAANOOMidiService.h"
#include "CAANOOSerialMidiDevice.h"

CAANOOMidiService::CAANOOMidiService() {
} ;

CAANOOMidiService::~CAANOOMidiService() {
} ;

void CAANOOMidiService::buildDriverList() {// Here we just loop over existing Midi out and create a midi device for each of them.

	Insert(new CAANOOSerialMidiDevice()) ;

} ;
