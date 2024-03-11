#include <stdint.h>
#include "picoTrackerAudioDriver.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Model/Config.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#include "audio_i2s.pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char picoTrackerAudioDriver::miniBlank_[MINI_BLANK_SIZE * 2 * sizeof(short)];

picoTrackerAudioDriver *picoTrackerAudioDriver::instance_ = NULL;
semaphore_t core1_audio;

static volatile unsigned long picoTracker_sound_pausei, picoTracker_exit;

void picoTracker_sound_pause(int yes) { picoTracker_sound_pausei = yes; }

// This calls comes after the call to the same function name in the pico audio
// driver
void __isr __time_critical_func(audio_i2s_dma_irq_handler)() {
  if (dma_irqn_get_channel_status(AUDIO_DMA_IRQ, AUDIO_DMA)) {
    dma_irqn_acknowledge_channel(AUDIO_DMA_IRQ, AUDIO_DMA);
    picoTrackerAudioDriver::IRQHandler();
  }
}

void picoTrackerAudioDriver::IRQHandler() { instance_->OnChunkDone(); }

void AudioThread() {
  // Allow core0 to pause this core when writing to flash
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_victim_init();
  while (true) {
    sem_acquire_blocking(&core1_audio);
    picoTrackerAudioDriver::BufferNeeded();
  }
}

void picoTrackerAudioDriver::BufferNeeded() {
  // Audio tick processes MIDI among other things
  // TODO: understand tick and buffer size relationship. currently not constant
  // probably not right
  // TODO: This could (should?) go into the main thread. If done tho, we geat a
  // deadlock in malloc mutex due to malloc being called from core1 and isr
  // simultaneously
  instance_->onAudioBufferTick();

  instance_->OnNewBufferNeeded();
}

picoTrackerAudioDriver::picoTrackerAudioDriver(AudioSettings &settings)
    : AudioDriver(settings) {

  isPlaying_ = false;
  picoTracker_exit = 0;
}

picoTrackerAudioDriver::~picoTrackerAudioDriver() { picoTracker_exit = 1; }

bool picoTrackerAudioDriver::InitDriver() {
  instance_ = this;

  // pico audio init

  // TODO: check what is this
  // i2s settings
  // DCDC PSM control
  // 0: PFM mode (best efficiency)
  // 1: PWM mode (improved ripple)
  gpio_init(PIN_DCDC_PSM_CTRL);
  gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
  gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

  // Setup GPIOs
  gpio_set_function(AUDIO_SDATA, GPIO_FUNC_PIO0);
  gpio_set_function(AUDIO_BCLK, GPIO_FUNC_PIO0);
  gpio_set_function(AUDIO_LRCLK, GPIO_FUNC_PIO0);

  // Claim and configure PIO0
  pio_sm_claim(AUDIO_PIO, AUDIO_SM);

  uint offset = pio_add_program(AUDIO_PIO, &audio_i2s_program);

  audio_i2s_program_init(AUDIO_PIO, AUDIO_SM, offset, AUDIO_SDATA, AUDIO_BCLK);

  // Claim and configure DMA
  dma_channel_claim(AUDIO_DMA);
  dma_channel_config dma_config = dma_channel_get_default_config(AUDIO_DMA);

  channel_config_set_dreq(&dma_config, DREQ_PIO0_TX0 + AUDIO_SM);
  channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
  channel_config_set_read_increment(&dma_config, true);
  dma_channel_configure(AUDIO_DMA, &dma_config,
                        &AUDIO_PIO->txf[AUDIO_SM], // dest
                        NULL,                      // src
                        0,                         // count
                        false                      // trigger
  );

  // Add our own callback func to run after the i2s irq func (priority 0x80)
  irq_set_exclusive_handler(DMA_IRQ_0 + AUDIO_DMA_IRQ,
                            audio_i2s_dma_irq_handler);
  dma_irqn_set_channel_enabled(AUDIO_DMA_IRQ, AUDIO_DMA, true);

  // Set PIO frequency
  uint32_t system_clock_frequency = clock_get_hz(clk_sys);
  int sample_freq = 44100;
  // This number is exactly 10000 for our 220.5MHz core freq
  uint32_t divider =
      system_clock_frequency * 2 / sample_freq; // avoid arithmetic overflow
  pio_sm_set_clkdiv_int_frac(AUDIO_PIO, AUDIO_SM, divider >> 8u,
                             divider & 0xffu);

  // Create mini blank buffer for underrun
  memset(miniBlank_, 0, MINI_BLANK_SIZE * 2 * sizeof(short));

  // Enable audio
  irq_set_enabled(DMA_IRQ_0 + AUDIO_DMA_IRQ, true);
  dma_channel_transfer_from_buffer_now(AUDIO_DMA, miniBlank_, MINI_BLANK_SIZE);
  pio_sm_set_enabled(AUDIO_PIO, AUDIO_SM, true);

  // Set Audio render thread on core1
  multicore_reset_core1();
  multicore_launch_core1(AudioThread);
  sem_init(&core1_audio, 0, SOUND_BUFFER_COUNT - 1);

  volume_ = 65;
  Config *config = Config::GetInstance();
  const char *volume = config->GetValue("VOLUME");

  if (volume) {
    volume_ = atoi(volume);
  }

  return true;
};

void picoTrackerAudioDriver::SetVolume(int v) {
  volume_ = (v <= 100) ? v : 100;
  Trace::Debug("Setting volume to %d", volume_);
};

int picoTrackerAudioDriver::GetVolume() { return volume_; };

void picoTrackerAudioDriver::CloseDriver() {

  pio_sm_set_enabled(AUDIO_PIO, AUDIO_SM, false);
  irq_set_enabled(DMA_IRQ_0 + AUDIO_DMA_IRQ, false);
  dma_irqn_set_channel_enabled(AUDIO_DMA_IRQ, AUDIO_DMA, false);
  irq_remove_handler(DMA_IRQ_0 + AUDIO_DMA_IRQ, audio_i2s_dma_irq_handler);
  dma_channel_unclaim(AUDIO_DMA);
  pio_sm_unclaim(AUDIO_PIO, AUDIO_SM);
  pio_clear_instruction_memory(AUDIO_PIO);
};

bool picoTrackerAudioDriver::StartDriver() {
  isPlaying_ = true;

  // Start filling up as many buffers as we have
  for (int i = 0; i < SOUND_BUFFER_COUNT - 1; i++) {
    sem_release(&core1_audio);
  }

  picoTracker_sound_pause(0);
  startTime_ = millis();

  return true;
};

void picoTrackerAudioDriver::StopDriver() {
  picoTracker_sound_pause(1);
  isPlaying_ = false;
};

void picoTrackerAudioDriver::OnChunkDone() {
  if (isPlaying_) {

    // Process MIDI
    MidiService::GetInstance()->Flush();

    // We got an IRQ so we know we finished playing from poolPlayPosition_
    // We mark it as empty and inspect the next buffer, if the buffer is not
    // empty, it means thread 2 finished processing that buffer and there are no
    // underruns Otherwise, we send a small blank buffer and wait for the other
    // thread to finish
    pool_[poolPlayPosition_].empty_ = true;

    int next = (poolPlayPosition_ + 1) % SOUND_BUFFER_COUNT;
    if (pool_[next].empty_) {
      dma_channel_transfer_from_buffer_now(AUDIO_DMA, miniBlank_,
                                           MINI_BLANK_SIZE);
    } else {
      poolPlayPosition_ = next;
      dma_channel_transfer_from_buffer_now(AUDIO_DMA,
                                           pool_[poolPlayPosition_].buffer_,
                                           pool_[poolPlayPosition_].size_ / 4);
    }

    // Finally we allow core1 to calculate an additional buffer
    sem_release(&core1_audio);
  }
}

int picoTrackerAudioDriver::GetPlayedBufferPercentage() {
  //	return
  // 100-(bufferSize_-bufferPos_-fragSize_)*100/(bufferSize_-fragSize_);
  // TODO: Do this right
  return 0;
};

double picoTrackerAudioDriver::GetStreamTime() {
  return (millis() - startTime_) / 1000.0;
};
