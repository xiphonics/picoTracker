#ifndef _W32_MIDI_H_
#define _W32_MIDI_H_

#include "Services/Midi/MidiOutDevice.h"
#include <windows.h>

#ifdef SendMessage
#undef SendMessage
#endif

class W32MidiDevice: public MidiOutDevice {
public:
	W32MidiDevice(const char *name) ;
	virtual ~W32MidiDevice() ;
	virtual bool Init() ;
	virtual void Close() ;
	virtual bool Start() ;
	virtual void Stop();
	virtual void SendMessage(MidiMessage &m);
private:
    HMIDIOUT midiOut_ ;
} ;
#endif
