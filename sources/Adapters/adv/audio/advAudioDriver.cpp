/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advAudioDriver.h"
#include "Adapters/adv/utils/utils.h"
#include "Application/Model/Config.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#include "platform.h"
#include "main.h" // has to come before FreeRTOS.h due to linkage of SystemCoreClock
#include "sai.h"
// Don't delete this space
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "tlv320aic3204.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern DMA_HandleTypeDef hdma_sai1_a;

// mini blank buffer for underrun, initialized to 0
const uint8_t advAudioDriver::miniBlank_[MINI_BLANK_SIZE] = {0};

advAudioDriver *advAudioDriver::instance_ = NULL;
SemaphoreHandle_t core1_audio;
static StaticSemaphore_t xSemaphoreBuffer;

static volatile unsigned long adv_sound_pausei, adv_exit;

void adv_sound_pause(int yes) { adv_sound_pausei = yes; }

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
  if (hsai->Instance == hsai_BlockA1.Instance) {
    advAudioDriver::IRQHandler();
  }
}

void advAudioDriver::IRQHandler() { instance_->OnChunkDone(); }

void AudioThread(void *) {
  while (true) {
    xSemaphoreTake(core1_audio, portMAX_DELAY);

    // Process MIDI
    MidiService::GetInstance()->Flush();

    advAudioDriver::BufferNeeded();
  }
}

void AudioOutput(void *) {
  while (true) {
    tlv320_select_output();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void advAudioDriver::BufferNeeded() {
  // Audio tick processes MIDI among other things
  // TODO: understand tick and buffer size relationship. currently not constant
  // probably not right
  // TODO: This could (should?) go into the main thread. If done tho, we geat a
  // deadlock in malloc mutex due to malloc being called from core1 and isr
  // simultaneously
  instance_->onAudioBufferTick();

  instance_->OnNewBufferNeeded();
}

advAudioDriver::advAudioDriver(AudioSettings &settings)
    : AudioDriver(settings) {

  isPlaying_ = false;
  adv_exit = 0;
}

advAudioDriver::~advAudioDriver() { adv_exit = 1; }

bool advAudioDriver::InitDriver() {
  instance_ = this;

  Config *config = Config::GetInstance();
  volume_ = 65;
  volume_ = config->GetValue("VOLUME");
  uint8_t outputLevel = config->GetValue("MASTER");

  // Configure codec
  tlv320_init();
  platform_set_output_level(outputLevel);
  // Takes some time between configuring HP detect and actually detecting
  vTaskDelay(pdMS_TO_TICKS(250));

  // Run a task that will periodically poll for output change. Cannot do this
  // easily on a timer due to de-pop wait times and timers being single
  // threaded. A delay on a timer can stall other timers.
  static StackType_t AudioOutputStack[1000];
  static StaticTask_t AudioOutputTCB;
  xTaskCreateStatic(AudioOutput, "Audio Output", 1000, NULL, 1,
                    AudioOutputStack, &AudioOutputTCB);

  core1_audio = xSemaphoreCreateCountingStatic(SOUND_BUFFER_COUNT - 1, 0,
                                               &xSemaphoreBuffer);

  static StackType_t AudioStack[4000];
  static StaticTask_t ProcessEventTCB;
  xTaskCreateStatic(AudioThread, "Audio", 4000, NULL, 6, AudioStack,
                    &ProcessEventTCB);

  return true;
};

void advAudioDriver::SetVolume(int v) {
  volume_ = (v <= 100) ? v : 100;
  Trace::Debug("Setting volume to %d", volume_);
};

int advAudioDriver::GetVolume() { return volume_; };

void advAudioDriver::CloseDriver(){
    // Not really used, maybe for sleep?
};

bool advAudioDriver::StartDriver() {
  isPlaying_ = true;

  // Start filling up as many buffers as we have
  for (int i = 0; i < SOUND_BUFFER_COUNT - 1; i++) {
    xSemaphoreGive(core1_audio);
  }

  // Send first DMA transfer
  HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)miniBlank_, MINI_BLANK_SIZE);

  adv_sound_pause(0);

  startTime_ = HAL_GetTick();

  tlv320_unmute();
  return true;
};

void advAudioDriver::StopDriver() {
  adv_sound_pause(1);
  isPlaying_ = false;
};

void advAudioDriver::OnChunkDone() {
  if (isPlaying_) {

    // We got an IRQ so we know we finished playing from poolPlayPosition_
    // We mark it as empty and inspect the next buffer, if the buffer is not
    // empty, it means thread 2 finished processing that buffer and there are no
    // underruns Otherwise, we send a small blank buffer and wait for the other
    // thread to finish
    pool_[poolPlayPosition_].empty_ = true;

    int next = (poolPlayPosition_ + 1) % SOUND_BUFFER_COUNT;
    if (pool_[next].empty_) {
      HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)miniBlank_,
                           MINI_BLANK_SIZE);
    } else {
      poolPlayPosition_ = next;
      HAL_SAI_Transmit_DMA(&hsai_BlockA1,
                           (uint8_t *)pool_[poolPlayPosition_].buffer_,
                           pool_[poolPlayPosition_].size_ / 2);
    }

    // Finally we allow core1 to calculate an additional buffer
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(core1_audio, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

int advAudioDriver::GetPlayedBufferPercentage() {
  //	return
  // 100-(bufferSize_-bufferPos_-fragSize_)*100/(bufferSize_-fragSize_);
  // TODO: Do this right
  return 0;
};

double advAudioDriver::GetStreamTime() {
  return (HAL_GetTick() - startTime_) / 1000.0;
};
