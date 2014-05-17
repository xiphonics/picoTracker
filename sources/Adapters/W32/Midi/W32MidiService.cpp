
#include "W32MidiService.h"
#include "W32MidiDEvice.h"
#include "System/io/Trace.h"

W32MidiService::W32MidiService() {
} ;

W32MidiService::~W32MidiService() {
} ;

void W32MidiService::buildDriverList() {// Here we just loop over existing Midi out and create a midi device for each of them.

	for (unsigned int i=0;i<midiOutGetNumDevs();i++) {
		MIDIOUTCAPS infos;
		int err = midiOutGetDevCaps (i, &infos, sizeof (MIDIOUTCAPS));
		if (err==MMSYSERR_NOERROR) {
			if (infos.dwSupport & MIDICAPS_STREAM) {
				W32MidiDevice *device=new W32MidiDevice(infos.szPname) ;
				Insert(device) ;
			}
		} ;
	} ;

} ;
