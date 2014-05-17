#ifndef _W32_MIDI_H_
#define _W32_MIDI_H_

#include "Services/Midi/MidiDevice.h"
#include <jack/jack.h>
#include "Adapters/Jack/Client/JackClient.h"
#include <string.h>

class JackMidiDevice: public MidiDevice {
public:
	JackMidiDevice(jack_client_t *client,const char *name) ;
	virtual ~JackMidiDevice() ;
	virtual bool Init() ;
	virtual void Close() ;
	virtual bool Start() ;
	virtual void Stop();
	virtual void SendMessage(MidiMessage &m);

	void SetFrameOffset(int offset) ;
	void OnNewFrame() ;
private:
	jack_client_t *client_ ;
	jack_port_t *port_ ;
	std::string portName_ ;
	int frameOffset_ ;
} ;
#endif
