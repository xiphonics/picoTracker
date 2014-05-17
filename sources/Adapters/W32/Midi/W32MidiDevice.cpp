
#include "W32MidiService.h"
#include "W32MidiDevice.h"
#include "System/io/Trace.h"


W32MidiDevice::W32MidiDevice(const char *name):MidiOutDevice(name) {
} ;

W32MidiDevice::~W32MidiDevice() {
} ;

bool W32MidiDevice::Init(){

	const char *deviceName=GetName() ;

	// Since the device order might have change, we recheck for the device ID

	int deviceID=-1 ;

	for (unsigned int i=0;i<midiOutGetNumDevs();i++) {
		MIDIOUTCAPS infos;
		int err = midiOutGetDevCaps (i, &infos, sizeof (MIDIOUTCAPS));
		if (err==MMSYSERR_NOERROR) {
			if (!strcmp(deviceName,infos.szPname)) {
				deviceID=i ;
				break ;
			} ;
		} ;
	} ;

	if (deviceID<0) {
		return false ;
	}

	int err = midiOutOpen(&midiOut_,
					deviceID,
					(DWORD_PTR)0,
					(DWORD_PTR)0,
					CALLBACK_NULL);           

	return (err==MMSYSERR_NOERROR) ;
} ;

void W32MidiDevice::Close(){
	midiOutClose(midiOut_) ;
	midiOut_=0 ;
}  ;

bool W32MidiDevice::Start(){
	return true ;
}  ;

void W32MidiDevice::Stop(){
} ;

void W32MidiDevice::SendMessage(MidiMessage &msg) {
	DWORD dwMsg=(msg.data2_<<16)+(msg.data1_<<8)+msg.status_ ;
	midiOutShortMsg(midiOut_,dwMsg) ;
} ;
