#ifndef _RT_MIDIIN_DEVICE_H_
#define _RT_MIDIIN_DEVICE_H_

#include "Services/Midi/MidiInDevice.h" 
#include "Externals/RtMidi/RtMidi.h"

class RTMidiInDevice:public MidiInDevice {
public:
	RTMidiInDevice(int index,const char *name) ;
	~RTMidiInDevice() ;

	void SendDriverMessage(MidiMessage &message) ;

private:

	// Driver specific initialisation

	virtual bool initDriver() ;
	virtual void closeDriver() ;
	virtual bool startDriver() ;
	virtual void stopDriver() ;

    RtMidiIn rtMidiIn_ ;
	int index_ ;
} ;
#endif