
#include "GP2XMidiService.h"
#include "GP2XSerialMidiDevice.h"

GP2XMidiService::GP2XMidiService() {
} ;

GP2XMidiService::~GP2XMidiService() {
} ;

void GP2XMidiService::buildDriverList() {// Here we just loop over existing Midi out and create a midi device for each of them.

	Insert(new GP2XSerialMidiDevice()) ;

} ;
