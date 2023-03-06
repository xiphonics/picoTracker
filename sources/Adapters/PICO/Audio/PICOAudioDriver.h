#ifndef _PICOAUDIO_DRIVER_H_
#define _PICOAUDIO_DRIVER_H_

#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h"
#include "System/Process/Process.h"

#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 26
#define PICO_AUDIO_I2S_DATA_PIN 28
#define PICO_AUDIO_I2S_PIO 0
#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)
#define GPIO_FUNC_PIOx __CONCAT(GPIO_FUNC_PIO, PICO_AUDIO_I2S_PIO)
#define DREQ_PIOx_TX0 __CONCAT(__CONCAT(DREQ_PIO, PICO_AUDIO_I2S_PIO), _TX0)
#define PICO_AUDIO_I2S_SM 0
#define PICO_AUDIO_I2S_DMA 0
#define PICO_AUDIO_I2S_DMA_IRQ 0
#define AUDIO_I2S

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

};
#endif
