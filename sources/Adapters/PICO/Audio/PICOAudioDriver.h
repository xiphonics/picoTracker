#ifndef _PICOAUDIO_DRIVER_H_
#define _PICOAUDIO_DRIVER_H_

#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h"
#include "System/Process/Process.h"

#define MINI_BLANK_SIZE 128

class PICOAudioDriver : public AudioDriver {
public:
  PICOAudioDriver(AudioSettings &settings);
  virtual ~PICOAudioDriver();

  // Sound implementation
  virtual bool InitDriver();
  virtual void CloseDriver();
  virtual bool StartDriver();
  virtual void StopDriver();
  virtual int GetPlayedBufferPercentage();
  virtual int GetSampleRate() { return 44100; };
  virtual bool Interlaced() { return true; };

  // Additional
  void OnChunkDone();
  void SetVolume(int v);
  int GetVolume();
  virtual double GetStreamTime();
  static void IRQHandler();

private:
  static PICOAudioDriver *instance_;

  AudioSettings settings_;
  static char miniBlank_[MINI_BLANK_SIZE * 2 * sizeof(short)];
  int volume_;
  int ticksBeforeMidi_;
  uint32_t startTime_;
};
#endif
