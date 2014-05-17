#ifndef _DUMMYAUDIO_H_
#define _DUMMYAUDIO_H_

#include "Services/Audio/Audio.h"

class DummyAudio: public Audio {
public:
       DummyAudio() ;
       ~DummyAudio() ;
       virtual void Init() ;
       virtual void Close() ;
};
#endif
