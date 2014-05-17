#ifndef _DINGOO_SERIAL_MIDI_DEVICE_H_
#define _DINGOO_SERIAL_MIDI_DEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

#include <termios.h>

class DINGOOSerialMidiDevice: public MidiOutDevice {
public:
	DINGOOSerialMidiDevice() ;
	virtual bool Init() ;
	virtual void Close() ;
	virtual bool Start() ;
	virtual void Stop();
	
protected:
	virtual void SendMessage(MidiMessage &) ;
private:
        struct termios oldtio_;
        int fd_ ;


} ;
#endif
