
#ifndef _RT_MIDI_SERVICE_H_
#define _W32_MIDI_SERVICE_H_

#include "Services/Midi/MidiService.h"
#include "Externals/RtMidi/RtMidi.h"

class RTMidiService: public MidiService {
public:
	RTMidiService() ;
	~RTMidiService() ;
protected:
	virtual void buildDriverList() ;
	int midiDelay_ ;
	RtMidiIn *rtMidiIn_ ;
	RtMidiOut *rtMidiOut_ ;
} ;

#endif