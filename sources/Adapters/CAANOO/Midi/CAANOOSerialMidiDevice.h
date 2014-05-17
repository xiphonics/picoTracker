#ifndef _CAANOO_SERIAL_MIDI_DEVICE_H_
#define _CAANOO_SERIAL_MIDI_DEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

#include <termios.h>

class CAANOOSerialMidiDevice: public MidiOutDevice {
public:
	CAANOOSerialMidiDevice() ;
	virtual bool Init() ;
	virtual void Close() ;
	virtual bool Start() ;
	virtual void Stop();
	
protected:
	virtual void SendMessage(MidiMessage &) ;
private:
        struct termios oldtio_;
        int fd_ ;
	const char *port_;

} ;
#endif
