
#include "DINGOOMidiService.h"
#include "DINGOOSerialMidiDevice.h"
#include "System/io/Trace.h"

DINGOOMidiService::DINGOOMidiService() {
} ;

DINGOOMidiService::~DINGOOMidiService() {
} ;

void DINGOOMidiService::buildDriverList() {// Here we just loop over existing Midi out and create a midi device for each of them.

	Insert(new DINGOOSerialMidiDevice()) ;

} ;
