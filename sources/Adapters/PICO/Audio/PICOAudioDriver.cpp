#include "PICOAudioDriver.h"
#include "Adapters/PICO/Utils/utils.h"
#include "Application/Model/Config.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#include <stdlib.h>
#include <string.h>

#include "hardware/irq.h"
#include "pico/audio.h"
#include <math.h>
#include <stdio.h>

// #define SAMPLES_PER_BUFFER 1156 // Samples / channel
#define SAMPLES_PER_BUFFER 256 // Samples / channel

PICOAudioDriver *PICOAudioDriver::instance_ = NULL;

static volatile unsigned long PICO_sound_pausei, PICO_exit;

void PICO_sound_pause(int yes) { PICO_sound_pausei = yes; }

// This calls comes after the call to the same function name in the pico audio
// driver
void __isr __time_critical_func(audio_i2s_dma_irq_handler)() {
  PICOAudioDriver::IRQHandler();
}

void PICOAudioDriver::IRQHandler() { instance_->OnChunkDone(); }

PICOAudioDriver::PICOAudioDriver(AudioSettings &settings)
    : AudioDriver(settings), miniBlank_(0) {

  isPlaying_ = false;

  spin_lock_ = spin_lock_init(spin_lock_claim_unused(true));

  PICO_exit = 0;

  audio_format_ = {.sample_freq = 44100,
                   .format = AUDIO_BUFFER_FORMAT_PCM_S16,
                   .channel_count = 2};

  static audio_buffer_format_t producer_format = {.format = &audio_format_,
                                                  .sample_stride = 4};

  ap_ = audio_new_producer_pool(&producer_format, 3,
                                SAMPLES_PER_BUFFER); // todo correct size

  i2s_config_ = {.data_pin = PICO_AUDIO_I2S_DATA_PIN,
                 .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
                 .dma_channel = 0,
                 .pio_sm = 0};
}

PICOAudioDriver::~PICOAudioDriver() { PICO_exit = 1; }

bool PICOAudioDriver::InitDriver() {
  instance_ = this;

  // pico audio init
  const audio_format_t *output_format;

  output_format = audio_i2s_setup(&audio_format_, &i2s_config_);
  if (!output_format) {
    Trace::Error("PicoAudio: Unable to open audio device");
    return false;
  }

  // Add our own callback func to run after the i2s irq func (priority 0x80)
  irq_add_shared_handler(DMA_IRQ_0 + PICO_AUDIO_I2S_DMA_IRQ,
                         audio_i2s_dma_irq_handler,
                         PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY - 0x40);

  bool ok = audio_i2s_connect(ap_);
  assert(ok);
  { // initial buffer data
    audio_buffer_t *buffer = take_audio_buffer(ap_, true);
    int16_t *samples = (int16_t *)buffer->buffer->bytes;
    for (uint i = 0; i < buffer->max_sample_count; i++) {
      samples[i * 2 + 0] = 0;
      samples[i * 2 + 1] = 0;
    }
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(ap_, buffer);
  }
  audio_i2s_set_enabled(true);

  volume_ = 65;
  Config *config = Config::GetInstance();
  const char *volume = config->GetValue("VOLUME");

  if (volume) {
    volume_ = atoi(volume);
  }

  // Create mini blank buffer for underrun
  miniBlank_ = (char *)malloc(SAMPLES_PER_BUFFER * 2 * sizeof(short));
  memset(miniBlank_, 0, SAMPLES_PER_BUFFER * 2 * sizeof(short));

  return true;
};

void PICOAudioDriver::SetVolume(int v) {
  volume_ = (v <= 100) ? v : 100;
  Trace::Debug("Setting volume to %d", volume_);
};

int PICOAudioDriver::GetVolume() { return volume_; };

void PICOAudioDriver::CloseDriver() {
  spin_lock_unclaim(spin_lock_get_num(spin_lock_));
  audio_i2s_set_enabled(false);
  if (miniBlank_) {
    SYS_FREE(miniBlank_);
    miniBlank_ = 0;
  }

  // TODO: proper hardware shudown and free resources?
  // If we do not do this at loading the driver again we don't have pio 0 for
  // use
};

bool PICOAudioDriver::StartDriver() {
  isPlaying_ = true;

  //  for (int i=0;i<settings_.preBufferCount_;i++) {
  //    AddBuffer((short *)miniBlank_,fragSize_/4) ;
  //  }
  if (settings_.preBufferCount_ == 0) {
    OnNewBufferNeeded();
  }

  ticksBeforeMidi_ = 4;

  for (int i = 0; i < ticksBeforeMidi_; i++) {
    AddBuffer((short *)miniBlank_, SAMPLES_PER_BUFFER * 2 * sizeof(short) / 4);
  }

  PICO_sound_pause(0);
  startTime_ = millis();

  return true;
};

void PICOAudioDriver::StopDriver() {
  PICO_sound_pause(1);
  isPlaying_ = false;
};

void PICOAudioDriver::OnChunkDone() {
  if (isPlaying_) {
    audio_buffer_t *buffer = take_audio_buffer(ap_, false);
    if (buffer == NULL) {
      printf("No buffer avail\n");
      return;
    }
    // Process MIDI
    if (ticksBeforeMidi_) {
      ticksBeforeMidi_--;
    } else {
      MidiService::GetInstance()->Flush();
    }
    if (buffer->max_sample_count >= pool_[poolPlayPosition_].size_ / 4) {
      memcpy(buffer->buffer->bytes, pool_[poolPlayPosition_].buffer_,
             pool_[poolPlayPosition_].size_);
      buffer->sample_count = pool_[poolPlayPosition_].size_ / 4;
      SYS_FREE(pool_[poolPlayPosition_].buffer_);
      pool_[poolPlayPosition_].buffer_ = 0;
      poolPlayPosition_ = (poolPlayPosition_ + 1) % SOUND_BUFFER_COUNT;
      OnNewBufferNeeded();
    } else {
      memcpy(buffer->buffer->bytes, pool_[poolPlayPosition_].buffer_,
             buffer->max_sample_count * 4);
      buffer->sample_count = buffer->max_sample_count;
      memmove(pool_[poolPlayPosition_].buffer_,
              pool_[poolPlayPosition_].buffer_ + (buffer->max_sample_count * 4),
              pool_[poolPlayPosition_].size_ - (buffer->max_sample_count * 4));
      pool_[poolPlayPosition_].size_ -= buffer->max_sample_count * 4;
    }
    give_audio_buffer(ap_, buffer);
    onAudioBufferTick();
  }
}

int PICOAudioDriver::GetPlayedBufferPercentage() {
  //	return
  //100-(bufferSize_-bufferPos_-fragSize_)*100/(bufferSize_-fragSize_);
  // TODO: Do this right
  return 0;
};

double PICOAudioDriver::GetStreamTime() {
  return (millis() - startTime_) / 1000.0;
};
