
#ifndef _CAANOO_MIDI_SERVICE_H_
#define _CAANOO_MIDI_SERVICE_H_

#include "Services/Midi/MidiService.h"

class CAANOOMidiService: public MidiService {
public:
	CAANOOMidiService() ;
	~CAANOOMidiService() ;
protected:
	virtual void buildDriverList() ;

} ;

#endif
