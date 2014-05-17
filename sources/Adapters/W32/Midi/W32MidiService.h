
#pragma once

#include "Services/Midi/MidiService.h"

class W32MidiService: public MidiService {
public:
	W32MidiService() ;
	~W32MidiService() ;
	void DelayedFlush(int samples) ;
protected:
	virtual void buildDriverList() ;
} ;
