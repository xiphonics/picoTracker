
#ifndef _GP2X_MIDI_SERVICE_H_
#define _GP2X_MIDI_SERVICE_H_

#include "Services/Midi/MidiService.h"

class GP2XMidiService: public MidiService {
public:
	GP2XMidiService() ;
	~GP2XMidiService() ;
protected:
	virtual void buildDriverList() ;

} ;

#endif
