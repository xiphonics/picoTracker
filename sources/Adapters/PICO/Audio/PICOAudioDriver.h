#ifndef _PICOAUDIO_DRIVER_H_
#define _PICOAUDIO_DRIVER_H_

#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 26
#define PICO_AUDIO_I2S_DATA_PIN 28
#define AUDIO_I2S

#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h"
#include "pico/audio_i2s.h"

#include "System/Process/Process.h"

// #define SOUND_BUFFER_COUNT 25

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
  char *miniBlank_;
  int volume_;
  int ticksBeforeMidi_;
  uint32_t startTime_;

  // pico driver specific
  audio_i2s_config_t i2s_config_;
  audio_format_t audio_format_;
  audio_buffer_pool_t *ap_;
  spin_lock_t *spin_lock_;
};
#endif
