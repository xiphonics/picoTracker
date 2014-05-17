
#ifndef _DINGOO_MIDI_SERVICE_H_
#define _DINGOO_MIDI_SERVICE_H_

#include "Services/Midi/MidiService.h"

class DINGOOMidiService: public MidiService {
public:
	DINGOOMidiService() ;
	~DINGOOMidiService() ;
protected:
	virtual void buildDriverList() ;

} ;

#endif
