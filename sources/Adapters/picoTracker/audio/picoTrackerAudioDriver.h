#ifndef _PICOTRACKERAUDIO_DRIVER_H_
#define _PICOTRACKERAUDIO_DRIVER_H_

#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h"
#include "System/Process/Process.h"

#define MINI_BLANK_SIZE 128

class picoTrackerAudioDriver : public AudioDriver {
public:
  picoTrackerAudioDriver(AudioSettings &settings);
  virtual ~picoTrackerAudioDriver();

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
  static void BufferNeeded();

private:
  static picoTrackerAudioDriver *instance_;

  AudioSettings settings_;
  static char miniBlank_[MINI_BLANK_SIZE * 2 * sizeof(short)];
  int volume_;
  int ticksBeforeMidi_;
  uint32_t startTime_;

  bool lastBufferGood_;
};
#endif
