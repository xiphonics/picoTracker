#ifndef _AUDIO_MODULE_H_
#define _AUDIO_MODULE_H_

#include "Application/Utils/fixed.h"
#include "Foundation/Types/Types.h"

class AudioModule {
public:
  virtual ~AudioModule(){};
  virtual bool Render(fixed *buffer, int samplecount) = 0;
};

#endif
