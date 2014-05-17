#ifndef _MIX_BUS_H_
#define _MIX_BUS_H_

#include "Services/Audio/AudioMixer.h"

class MixBus: public AudioMixer {
public:
	MixBus():AudioMixer("bus") {} ;
	virtual ~MixBus() {} ;
} ;
#endif
