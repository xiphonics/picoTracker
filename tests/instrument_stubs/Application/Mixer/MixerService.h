#pragma once
#include "Application/Mixer/MixBus.h"

// Channel tests do not connect to an audio output bus.
class MixerService {
public:
  static MixerService *GetInstance() {
    static MixerService mixer;
    return &mixer;
  }
  MixBus *GetMixBus(int32_t) { return nullptr; }
};
