#ifndef _DUMMY_MIDI_H_
#define _DUMMY_MIDI_H_

#include "Services/Midi/MidiService.h"

class DummyMidi: public MidiService {
public:
	DummyMidi()  ;
	~DummyMidi()  ;
	virtual void buildDriverList() ;
	
} ;
#endif
