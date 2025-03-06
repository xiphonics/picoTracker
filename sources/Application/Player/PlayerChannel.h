
#ifndef _PLAYER_CHANNEL_H_
#define _PLAYER_CHANNEL_H_

#include "Application/Instruments/I_Instrument.h"
#include "Application/Mixer/MixBus.h"
#include "Services/Audio/AudioModule.h"

class PlayerChannel : public AudioModule {
public:
  PlayerChannel(int index);
  virtual ~PlayerChannel();
  virtual fixed Render(fixed *buffer, int samplecount);
  void StartInstrument(I_Instrument *instr, unsigned char note,
                       bool cleanStart);
  void StopInstrument();
  I_Instrument *GetInstrument();
  void SetMute(bool muted);
  bool IsMuted();
  void SetMixBus(int i);
  void Reset();

private:
  int index_;
  I_Instrument *instr_;
  bool muted_;
  int busIndex_;
  MixBus *mixBus_;
};

#endif
