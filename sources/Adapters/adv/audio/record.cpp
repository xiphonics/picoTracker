#include "record.h"
#include "Application/Instruments/WavHeader.h"
#include "Application/Player/Player.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "sai.h"
#include "sd_diskio.h"
#include "tlv320aic3204.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

static FileHandle RecordFile;

uint8_t *activeBuffer;
uint8_t *writeBuffer;

__attribute__((section(".FRAMEBUFFER"))) __attribute__((aligned(32)))
uint16_t recordBuffer[RECORD_BUFFER_SIZE];
// TODO (democloid): this is less than ideal, but works for now. we need to swap
// the samples in the input buffer and since we also do the monitoring from it,
// we cannot invert it in place (and we would have to also invert the monitoring
// playback). Look for a better solution Half-buffer sized swap workspace for
// writing to disk
__attribute__((section(".FRAMEBUFFER"))) __attribute__((
    aligned(32))) static uint16_t recordSwapBuffer[RECORD_BUFFER_SIZE / 2];

TaskHandle_t RecordHandle = NULL;

static volatile bool recordingActive = false;
static volatile bool writeInProgress = false;

static volatile bool monitoringOnly = false;
static volatile uint8_t buffersToSkip = 0;

static int lineInGainDb = 0;
static int micGainDb = 0;

bool IsRecordingActive() { return recordingActive; }

// Tracking which half is ready
static volatile bool isHalfBuffer = false;
static volatile bool thresholdOK = false;
static bool first_pass = true;
static uint32_t start = 0;
static uint32_t totalSamplesWritten = 0;
static uint32_t recordDuration_ = MAX_INT32;
static RecordSource source_ = LineIn;

// Swap L/R channels for interleaved 16-bit stereo using a 32-bit rotate.
// 'size' is the number of uint16_t entries; assumes even (multiple of 2).
static inline void SwapChannelsToBuffer(const uint16_t *src, uint16_t *dst,
                                        uint32_t size) {
  uint32_t words = size >> 1; // two samples per 32-bit word
  const uint32_t *src32 = reinterpret_cast<const uint32_t *>(src);
  uint32_t *dst32 = reinterpret_cast<uint32_t *>(dst);
  for (uint32_t i = 0; i < words; ++i) {
    uint32_t lr = src32[i];
    // Rotate by 16 bits: [RRRR][LLLL] -> [LLLL][RRRR]
    dst32[i] = (lr >> 16) | (lr << 16);
  }
}

StaticSemaphore_t xSemaphoreBuffer;
SemaphoreHandle_t recordingFinishedSemaphore = NULL;

void SetInputSource(RecordSource source) {
  source_ = source;
  switch (source) {
  case LineIn:
    tlv320_disable_mic();
    tlv320_enable_linein();
    tlv320_set_linein_gain_db(lineInGainDb);
    break;
  case Mic:
    tlv320_disable_linein();
    tlv320_enable_mic();
    tlv320_set_mic_gain_db(micGainDb);
    break;
  case USBIn:
    // TODO:
    NAssert(false);
    break;
  case AllOff:
    // default to all off
    tlv320_disable_linein();
    tlv320_disable_mic();
    break;
  }
}

void SetLineInGain(uint8_t gainDb) {
  lineInGainDb = gainDb;
  if (source_ == LineIn) {
    tlv320_set_linein_gain_db(lineInGainDb);
  }
}

void SetMicGain(uint8_t gainDb) {
  micGainDb = gainDb;
  if (source_ == Mic) {
    tlv320_set_mic_gain_db(micGainDb);
  }
}

void StartMonitoring() {

  // Set the flag for monitoring-only mode
  monitoringOnly = true;
  recordingActive = true; // Use this to keep the task loop active
  first_pass = true;      // Ensure StartRecordStreaming is called

  // Clear the buffer and start the SAI DMA
  memset(recordBuffer, 0, RECORD_BUFFER_SIZE * sizeof(uint16_t));
  HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)recordBuffer,
                      RECORD_BUFFER_SIZE);

  // Wake up the Record task to handle the stream
  vTaskResume(RecordHandle);
  Trace::Log("RECORD", "Monitoring started");
}

void StopMonitoring() { // Use the main 'recordingActive' flag to trigger the
                        // task's cleanup routine
  recordingActive = false;
  monitoringOnly = false; // Also reset our monitoring flag

  // Notify the Record task to wake it from its wait state so it can clean up
  if (RecordHandle != NULL) {
    xTaskNotifyGive(RecordHandle);
  }
  // disable all inputs when we stop monitoring
  tlv320_disable_linein();
  tlv320_disable_mic();
  Trace::Log("RECORD", "Monitoring stopped");
}

bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds) {
  // TODO 0 milliseconds indicates unlimited duration recording
  if (milliseconds != 0) {
    recordDuration_ = milliseconds;
  } else {
    recordDuration_ = MAX_INT32;
  }

  auto fs = FileSystem::GetInstance();
  fs->chdir(RECORDINGS_DIR);

  // Create or truncate the file using FileSystem
  RecordFile = fs->Open(filename, "w");
  if (!RecordFile) {
    Trace::Log("RECORD", "Could not create file");
    return false;
  }

  // Write WAV header (44.1kHz, 16-bit, stereo)
  if (!WavHeaderWriter::WriteHeader(RecordFile.get(), 44100, 2, 16)) {
    Trace::Log("RECORD", "Failed to write WAV header");
    RecordFile.reset();
    return false;
  }

  // only set if recording file could be opened
  monitoringOnly = false;
  thresholdOK = false;
  buffersToSkip = 2; // drop monitoring buffers from before we start recording
  first_pass = true;
  recordingFinishedSemaphore = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);

  // Reset sample counter
  totalSamplesWritten = 0;

  // Signal task to start recording
  recordingActive = true;

  // Resume or create task if needed
  vTaskResume(RecordHandle);

  // Clear / init buffers
  memset(recordBuffer, 0, RECORD_BUFFER_SIZE * sizeof(uint16_t));

  // Start first DMA transfer
  auto status = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)recordBuffer,
                                    RECORD_BUFFER_SIZE);
  if (status != HAL_OK) {
    Trace::Error("SAI receive failed");
  }
  start = xTaskGetTickCount();
  return true;
}

void StopRecording() {
  if (!recordingActive) {
    return; // Already stopping/stopped
  }

  recordingActive = false;

  // Notify the Record task to wake it up
  if (RecordHandle != NULL) {
    xTaskNotifyGive(RecordHandle);
  }

  // Now, wait here until the Record task signals it's done
  if (recordingFinishedSemaphore != NULL) {
    xSemaphoreTake(recordingFinishedSemaphore, portMAX_DELAY);
  }
  // disable all inputs when we finish recording
  tlv320_disable_linein();
  tlv320_disable_mic();
}

void Record(void *) {
  UINT bw;
  uint32_t lastLoggedTime = xTaskGetTickCount();
  for (;;) {
    if (xTaskGetTickCount() - start > recordDuration_) {
      StopRecording();
    }

    if (!recordingActive) {
      HAL_SAI_DMAStop(&hsai_BlockB1);

      if (RecordFile) {
        Trace::Log("RECORD", "About to update WAV header");
        // Update WAV header with final file size
        if (!WavHeaderWriter::UpdateFileSize(RecordFile.get(),
                                             totalSamplesWritten)) {
          Trace::Log("RECORD", "Failed to update WAV header");
        }

        Trace::Log("RECORD", "STOP Recording: dur:%d samples:%d",
                   (xTaskGetTickCount() - start), totalSamplesWritten);

        RecordFile.reset();
      }

      Player::GetInstance()->StopRecordStreaming();

      if (recordingFinishedSemaphore != NULL) {
        xSemaphoreGive(recordingFinishedSemaphore);
      }

      vTaskSuspend(nullptr); // Suspend self until StartRecording resumes it
    }

    // Interrupt return
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // DMA transmit finished, dump to file
    // Tracking which half is ready
    uint32_t offset = 0;
    if (!isHalfBuffer) {
      offset = RECORD_BUFFER_SIZE / 2;
    }

    const uint32_t halfSamples = RECORD_BUFFER_SIZE / 2; // uint16_t entries
    SwapChannelsToBuffer(recordBuffer + offset, recordSwapBuffer, halfSamples);
    // Write raw audio data (uint16_t samples as bytes)
    if (!monitoringOnly) {
      // When we monitor we are continously recording, so there is data before
      // we start recording. We skip a couple of buffers to actually record from
      // after the moment we press the recording button
      if (buffersToSkip > 0) {
        buffersToSkip = buffersToSkip - 1;
        continue;
      }
      if (RecordFile) {
        writeInProgress = true;
        // Write swapped half-buffer to disk (bytes = samples * 2)
        const uint32_t bytesToWrite = halfSamples * sizeof(uint16_t);
        int bytesWritten =
            RecordFile->Write((uint8_t *)recordSwapBuffer, bytesToWrite, 1);
        // sync immediately after writing the buffer for consistent if not
        // fastest perf
        RecordFile->Sync();
        writeInProgress = false;
        if (bytesWritten != (int)bytesToWrite) {
          Trace::Error("write failed\r\n");
          // Stop recording so the task can clean up and signal completion.
          recordingActive = false;
          continue;
        } else {
          // Total samples written totalSamplesWritten/4 for stereo 16bit
          // samples
          totalSamplesWritten += bytesWritten / 4;

          auto now = xTaskGetTickCount();
          if ((now - lastLoggedTime) > 1000) { // log every sec
            Trace::Debug("RECORDING [%i]s [%d]",
                         (xTaskGetTickCount() - start) / 1000,
                         totalSamplesWritten);
            lastLoggedTime = now;
          }
        }
      } else {
        writeInProgress = false;
        Trace::Error("RecordFile is null");
      }
    }
    // start playing
    if (first_pass) {
      first_pass = false;
      Player::GetInstance()->StartRecordStreaming(recordBuffer,
                                                  RECORD_BUFFER_SIZE, true);
    }
  }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  isHalfBuffer = true;
  if (writeInProgress) {
    Trace::Error("RECORD - SD Card write underrun!");
  }
  vTaskNotifyGiveFromISR(RecordHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  isHalfBuffer = false;
  if (writeInProgress) {
    Trace::Error("RECORD - SD Card write underrun!");
  }
  vTaskNotifyGiveFromISR(RecordHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
