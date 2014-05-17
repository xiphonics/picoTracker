#pragma once

#include "ControllerSource.h"

#define MAX_HAT_CHANNELS 4 // up to 4 hats

class HatControllerSource: public ControllerSource {
public:
	HatControllerSource(const char *name) ;
	virtual ~HatControllerSource() ;
	virtual Channel *GetChannel(const char *url) ;
	virtual bool IsRunning() { return true ; } ;
	void SetHat(int which,int bitmask) ;
private:
	Channel channel_[MAX_HAT_CHANNELS*4] ; // 4 directions
} ;
