#include "HatControllerSource.h"
#include "System/Console/Trace.h"
#include <cstring>

HatControllerSource::HatControllerSource(const char *name):ControllerSource("hat",name) {
} ;

HatControllerSource::~HatControllerSource() {
} ;

Channel *HatControllerSource::GetChannel(const char *url) {
	if (strlen(url)<3) return 0 ;
	int which=url[0]-'0' ;
	int axis=url[2]-'0' ;
	return channel_+which*4+axis ;
	//int bitmask=(url[1]=='+')?1:0 ;
	//return channel_+axis*2+side ;
}

// value = -1 -> 1

void HatControllerSource::SetHat(int which,int bitmask) {
	int base=which*4 ;

	channel_[base].SetValue(bitmask&0x01?1.0f:0.0f) ;
	channel_[base+1].SetValue(bitmask&0x02?1.0f:0.0f) ;
	channel_[base+2].SetValue(bitmask&0x04?1.0f:0.0f) ;
	channel_[base+3].SetValue(bitmask&0x08?1.0f:0.0f) ;
	channel_[base].NotifyObservers() ;
	channel_[base+1].NotifyObservers() ;
	channel_[base+2].NotifyObservers() ;
	channel_[base+3].NotifyObservers() ;
}
