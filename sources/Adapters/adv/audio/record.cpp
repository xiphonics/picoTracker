#include "record.h"
#include "Application/Instruments/WavHeader.h"
#include "Application/Persistency/PersistenceConstants.h"
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

static constexpr uint32_t kRecordSampleRate = 44100;
static constexpr uint32_t kRecordRamSeconds = 30;
static constexpr uint32_t kRecordRamSamples =
    kRecordSampleRate * kRecordRamSeconds * 2; // max uint16_t entries
static constexpr uint32_t kRecordDumpChunkBytes = 4096;
static constexpr uint32_t kStopRecordingTimeoutMs = 30000;
// On ADV hardware, mic signal is present on the even interleaved lane.
static constexpr uint8_t kMicInputLaneIndex = 0;

__attribute__((section(".SDRAM1"))) __attribute__((aligned(32)))
uint16_t recordBuffer[RECORD_BUFFER_SIZE];
__attribute__((section(".SDRAM1")))
__attribute__((aligned(32))) static uint16_t recordRamBuffer[kRecordRamSamples];

TaskHandle_t RecordHandle = NULL;

static volatile bool recordingActive = false;
static volatile bool writeInProgress = false;
static volatile bool savingInProgress = false;
static volatile uint8_t savingProgressPercent = 0;
// True only when the latest recording was successfully persisted to disk.
static volatile bool lastRecordingSavedAudio = false;
// True only for explicit recording sessions (not monitor-only streaming).
static volatile bool recordingSessionActive = false;

static volatile bool monitoringOnly = false;
static volatile uint8_t buffersToSkip = 0;

static int lineInGainDb = 0;
static int micGainDb = 0;

static inline void SetRecordMonitorAudioActive(bool active) {
  // Keep this local to record/monitor flow and avoid changing core player
  // behavior. If the sequencer is running, player owns audio-active state.
  Player *player = Player::GetInstance();
  if (player && !player->IsRunning()) {
    tlv320_set_audio_output_active(active);
  }
}

bool IsRecordingActive() { return recordingActive; }
bool IsSavingRecording() { return savingInProgress; }
uint8_t GetSavingProgressPercent() { return savingProgressPercent; }
bool DidLastRecordingCaptureAudio() { return lastRecordingSavedAudio; }

// Tracking which half is ready
static volatile bool isHalfBuffer = false;
static bool first_pass = true;
static uint32_t start = 0;
static uint32_t totalSamplesWritten = 0;
static uint32_t recordDuration_ = MAX_INT32;
static RecordSource source_ = LineIn;
static uint32_t recordRamSamples = 0;
static char recordingFilename_[PFILENAME_SIZE] = {0};
static uint16_t recordChannelCount = 2;

static bool DeleteRecordingFile(const char *filename) {
  if (!filename || filename[0] == '\0') {
    return false;
  }
  auto fs = FileSystem::GetInstance();
  if (!fs || !fs->chdir(RECORDINGS_DIR)) {
    Trace::Error("RECORD: could not access recording directory %s",
                 RECORDINGS_DIR);
    return false;
  }
  if (!fs->DeleteFile(filename)) {
    Trace::Error("RECORD: failed to delete file %s", filename);
    return false;
  }
  return true;
}

static bool PersistRecordingToWav(const char *filename, const uint8_t *data,
                                  uint32_t bytesToWrite, uint16_t channelCount,
                                  uint32_t *outFramesWritten) {
  if (outFramesWritten) {
    *outFramesWritten = 0;
  }
  if (!filename || filename[0] == '\0') {
    Trace::Error("RECORD: invalid recording filename");
    return false;
  }

  auto fs = FileSystem::GetInstance();
  if (!fs || !fs->chdir(RECORDINGS_DIR)) {
    Trace::Error("RECORD: could not access recording directory %s",
                 RECORDINGS_DIR);
    return false;
  }

  RecordFile = fs->Open(filename, "w");
  if (!RecordFile) {
    Trace::Error("RECORD: could not create file %s", filename);
    return false;
  }

  if (!WavHeaderWriter::WriteHeader(RecordFile.get(), kRecordSampleRate,
                                    channelCount, 16)) {
    Trace::Error("RECORD: failed to write WAV header");
    RecordFile.reset();
    DeleteRecordingFile(filename);
    return false;
  }

  uint32_t bytesWrittenTotal = 0;
  writeInProgress = true;
  while (bytesWrittenTotal < bytesToWrite) {
    uint32_t chunk = bytesToWrite - bytesWrittenTotal;
    if (chunk > kRecordDumpChunkBytes) {
      chunk = kRecordDumpChunkBytes;
    }
    int bytesWritten = RecordFile->Write(data + bytesWrittenTotal, chunk, 1);
    if (bytesWritten != (int)chunk) {
      Trace::Error("RECORD: write failed at %lu/%lu bytes (file err=%d)",
                   bytesWrittenTotal, bytesToWrite, RecordFile->Error());
      writeInProgress = false;
      RecordFile.reset();
      DeleteRecordingFile(filename);
      return false;
    }
    bytesWrittenTotal += chunk;
    savingProgressPercent = (bytesWrittenTotal * 100U) / bytesToWrite;
  }

  if (!RecordFile->Sync()) {
    Trace::Error("RECORD: sync failed while finalizing recording");
    writeInProgress = false;
    RecordFile.reset();
    DeleteRecordingFile(filename);
    return false;
  }
  writeInProgress = false;

  uint32_t frameBytes = static_cast<uint32_t>(channelCount) * sizeof(uint16_t);
  uint32_t framesWritten =
      (frameBytes > 0) ? (bytesWrittenTotal / frameBytes) : 0;
  if (!WavHeaderWriter::UpdateFileSize(RecordFile.get(), framesWritten,
                                       channelCount, 2)) {
    Trace::Error("RECORD: failed to update WAV header");
    RecordFile.reset();
    DeleteRecordingFile(filename);
    return false;
  }

  RecordFile.reset();
  if (outFramesWritten) {
    *outFramesWritten = framesWritten;
  }
  return true;
}

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
  recordingSessionActive = false;
  // Monitoring should not inherit a prior recording timeout.
  recordDuration_ = MAX_INT32;
  start = xTaskGetTickCount();
  recordingActive = true; // Use this to keep the task loop active
  first_pass = true;      // Ensure StartRecordStreaming is called
  SetRecordMonitorAudioActive(true);

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
  if (RecordFile) {
    Trace::Error("RECORD: stale file handle detected before start, closing");
    RecordFile.reset();
  }
  recordingSessionActive = false;

  // TODO 0 milliseconds indicates unlimited duration recording
  if (milliseconds != 0) {
    recordDuration_ = pdMS_TO_TICKS(milliseconds);
  } else {
    recordDuration_ = pdMS_TO_TICKS(kRecordRamSeconds * 1000);
  }

  if (filename && filename[0] != '\0') {
    snprintf(recordingFilename_, sizeof(recordingFilename_), "%s", filename);
  } else {
    Trace::Error("RECORD: invalid recording filename");
    recordingFilename_[0] = '\0';
    return false;
  }

  recordChannelCount = (source_ == Mic) ? 1 : 2;

  // Activate record mode.
  monitoringOnly = false;
  recordingSessionActive = true;
  buffersToSkip = 2; // drop monitoring buffers from before we start recording
  first_pass = true;
  recordingFinishedSemaphore = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);

  // Reset sample counter
  totalSamplesWritten = 0;
  recordRamSamples = 0;
  lastRecordingSavedAudio = false;
  savingInProgress = false;
  savingProgressPercent = 0;

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
  // Idempotent: if neither active recording nor save finalization is running,
  // there is nothing to wait for.
  if (!recordingActive && !savingInProgress) {
    FinishStopRecording();
    return;
  }

  if (recordingActive) {
    RequestStopRecording();
  }

  // Block for a bounded amount of time while recording data is flushed to SD.
  if (!WaitForRecordingStop(kStopRecordingTimeoutMs)) {
    Trace::Error("RECORD: stop timed out after %lu ms",
                 kStopRecordingTimeoutMs);
  }
  FinishStopRecording();
}

void RequestStopRecording() {
  if (!recordingActive) {
    return;
  }

  recordingActive = false;

  // Notify the Record task to wake it up
  if (RecordHandle != NULL) {
    xTaskNotifyGive(RecordHandle);
  }
}

bool WaitForRecordingStop(uint32_t timeoutMs) {
  if (recordingFinishedSemaphore == NULL) {
    return true;
  }

  if (timeoutMs == MAX_INT32) {
    return xSemaphoreTake(recordingFinishedSemaphore, portMAX_DELAY) == pdTRUE;
  }

  return xSemaphoreTake(recordingFinishedSemaphore, pdMS_TO_TICKS(timeoutMs)) ==
         pdTRUE;
}

void FinishStopRecording() {
  // disable all inputs when we finish recording
  tlv320_disable_linein();
  tlv320_disable_mic();
}

void Record(void *) {
  uint32_t lastLoggedTime = xTaskGetTickCount();
  for (;;) {
    if (!monitoringOnly && (xTaskGetTickCount() - start > recordDuration_)) {
      RequestStopRecording();
    }

    if (!recordingActive) {
      HAL_SAI_DMAStop(&hsai_BlockB1);

      Player::GetInstance()->StopRecordStreaming();
      SetRecordMonitorAudioActive(false);

      bool writeFailed = false;
      if (recordingSessionActive) {
        if (recordRamSamples > 0) {
          savingInProgress = true;
          savingProgressPercent = 0;
          const uint32_t bytesToWrite = recordRamSamples * sizeof(uint16_t);
          uint32_t framesWritten = 0;
          const uint8_t *data =
              reinterpret_cast<const uint8_t *>(recordRamBuffer);
          bool saved =
              PersistRecordingToWav(recordingFilename_, data, bytesToWrite,
                                    recordChannelCount, &framesWritten);
          totalSamplesWritten = framesWritten;
          lastRecordingSavedAudio = saved;
          writeFailed = !saved;
          if (saved) {
            savingProgressPercent = 100;
          }
          savingInProgress = false;
        } else {
          // No captured audio: treat this as cancelled (no file created).
          lastRecordingSavedAudio = false;
          totalSamplesWritten = 0;
        }

        recordingSessionActive = false;

        Trace::Log("RECORD", "STOP Recording: dur:%d samples:%d status:%s",
                   (xTaskGetTickCount() - start), totalSamplesWritten,
                   writeFailed ? "FAILED" : "OK");
      }

      if (recordingFinishedSemaphore != NULL) {
        xSemaphoreGive(recordingFinishedSemaphore);
      }

      vTaskSuspend(nullptr); // Suspend self until StartRecording resumes it
    }

    // Interrupt return
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // A stop request also wakes this task. In that case, skip buffer processing
    // and let the next loop iteration run the stop/finalize path immediately.
    if (!recordingActive) {
      continue;
    }
    // DMA transmit finished, dump to file
    // Tracking which half is ready
    uint32_t offset = 0;
    if (!isHalfBuffer) {
      offset = RECORD_BUFFER_SIZE / 2;
    }

    const uint32_t halfSamples = RECORD_BUFFER_SIZE / 2; // uint16_t entries
    if (!monitoringOnly) {
      // When we monitor we are continously recording, so there is data before
      // we start recording. We skip a couple of buffers to actually record from
      // after the moment we press the recording button
      if (buffersToSkip > 0) {
        buffersToSkip = buffersToSkip - 1;
        continue;
      }
      uint32_t remainingSamples = kRecordRamSamples - recordRamSamples;
      if (remainingSamples == 0) {
        Trace::Log("RECORD", "RAM buffer full");
        recordingActive = false;
      } else {
        if (recordChannelCount == 1) {
          // Mic source is recorded as mono WAV and always uses the even lane
          // on ADV interleaved input.
          const uint32_t halfFrames = halfSamples / 2;
          uint32_t copyFrames = halfFrames;
          if (copyFrames > remainingSamples) {
            copyFrames = remainingSamples;
          }

          for (uint32_t i = 0; i < copyFrames; ++i) {
            recordRamBuffer[recordRamSamples + i] =
                recordBuffer[offset + i * 2 + kMicInputLaneIndex];
          }
          recordRamSamples += copyFrames;
          totalSamplesWritten = recordRamSamples;

          if (copyFrames < halfFrames) {
            Trace::Log("RECORD", "RAM buffer full");
            recordingActive = false;
          }
        } else {
          uint32_t copySamples = halfSamples;
          if (copySamples > remainingSamples) {
            copySamples = remainingSamples;
          }

          SwapChannelsToBuffer(recordBuffer + offset,
                               recordRamBuffer + recordRamSamples, copySamples);
          recordRamSamples += copySamples;
          totalSamplesWritten = recordRamSamples / 2;

          if (copySamples < halfSamples) {
            Trace::Log("RECORD", "RAM buffer full");
            recordingActive = false;
          }
        }

        auto now = xTaskGetTickCount();
        if ((now - lastLoggedTime) > 1000) { // log every sec
          Trace::Debug("RECORDING [%i]s [%d]",
                       (xTaskGetTickCount() - start) / 1000,
                       totalSamplesWritten);
          lastLoggedTime = now;
        }
      }
    }
    // start playing
    if (first_pass) {
      first_pass = false;
      SetRecordMonitorAudioActive(true);
      Player::GetInstance()->StartRecordStreaming(recordBuffer,
                                                  RECORD_BUFFER_SIZE, true);
    }
  }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  isHalfBuffer = true;
  if (writeInProgress) {
    Trace::Error("RECORD - write underrun!");
  }
  vTaskNotifyGiveFromISR(RecordHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  isHalfBuffer = false;
  if (writeInProgress) {
    Trace::Error("RECORD - write underrun!");
  }
  vTaskNotifyGiveFromISR(RecordHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
