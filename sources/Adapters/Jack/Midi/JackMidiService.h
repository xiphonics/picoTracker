
#ifndef _JACK_MIDI_SERVICE_H_
#define _JACK_MIDI_SERVICE_H_

#include "Services/Midi/MidiService.h"

class JackMidiService: public MidiService {
public:
	JackMidiService() ;
	~JackMidiService() ;
	void SetFrameOffset(int offset) ;
	void OnNewFrame() ;
protected:
	virtual void buildDriverList() ;

} ;

#endif