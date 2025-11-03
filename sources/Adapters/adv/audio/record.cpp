#include "record.h"
#include "Application/Instruments/WavHeader.h"
#include "Application/Player/Player.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "sai.h"
#include "sd_diskio.h"
#include "tlv320aic3204.h"
#include <cstdio>
#include <cstring>

static I_File *RecordFile = nullptr;

uint8_t *activeBuffer;
uint8_t *writeBuffer;

__attribute__((section(".FRAMEBUFFER"))) __attribute__((aligned(32)))
uint16_t recordBuffer[RECORD_BUFFER_SIZE];

TaskHandle_t RecordHandle = NULL;

static volatile bool recordingActive = false;
static volatile bool writeInProgress = false;

static volatile bool g_monitoringOnly = false;

bool IsRecordingActive() { return recordingActive; }

// Tracking which half is ready
static volatile bool isHalfBuffer = false;
static volatile bool thresholdOK = false;
static bool first_pass = true;
static uint32_t start = 0;
static uint32_t totalSamplesWritten = 0;
static uint32_t recordDuration_ = MAX_INT32;
static RecordSource source_ = LineIn;

StaticSemaphore_t xSemaphoreBuffer;
SemaphoreHandle_t g_recordingFinishedSemaphore = NULL;

void SetInputSource(RecordSource source) {
  source_ = source;
  switch (source) {
  case LineIn:
    tlv320_disable_mic();
    tlv320_enable_linein();
    break;
  case Mic:
    tlv320_disable_linein();
    tlv320_enable_mic();
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

void StartMonitoring() {

  // Set the flag for monitoring-only mode
  g_monitoringOnly = true;
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
  g_monitoringOnly = false; // Also reset our monitoring flag

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
  if (!WavHeaderWriter::WriteHeader(RecordFile, 44100, 2, 16)) {
    Trace::Log("RECORD", "Failed to write WAV header");
    RecordFile->Close();
    RecordFile = nullptr;
    return false;
  }

  // only set if recording file could be opened
  g_monitoringOnly = false;
  thresholdOK = false;
  first_pass = true;
  g_recordingFinishedSemaphore =
      xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);

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
  if (g_recordingFinishedSemaphore != NULL) {
    xSemaphoreTake(g_recordingFinishedSemaphore, portMAX_DELAY);
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
        if (!WavHeaderWriter::UpdateFileSize(RecordFile, totalSamplesWritten)) {
          Trace::Log("RECORD", "Failed to update WAV header");
        }

        Trace::Log("RECORD", "STOP Recording: dur:%d samples:%d",
                   (xTaskGetTickCount() - start), totalSamplesWritten);

        // Close file
        if (!RecordFile->Close()) {
          Trace::Error("failed to close recording file");
        }
        RecordFile = nullptr;
      }

      Player::GetInstance()->StopRecordStreaming();

      if (g_recordingFinishedSemaphore != NULL) {
        xSemaphoreGive(g_recordingFinishedSemaphore);
      }

      vTaskSuspend(nullptr); // Suspend self until StartRecording resumes it
    }

    // Interrupt return
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // DMA transmit finished, dump to file
    // Tracking which half is ready
    uint32_t offset = 0;
    if (isHalfBuffer) {
      offset = RECORD_BUFFER_SIZE / 2;
    }
    /*    if (!thresholdOK) {
      // half the buffer in uint16_t
      for (uint32_t i = 0; i < RECORD_BUFFER_SIZE / 2; ++i) {
        if (abs(buffer[i]) > threshold) {
          thresholdOK = true;
          break;
        }
      }
      }*/
    // Write raw audio data (uint16_t samples as bytes)
    if (!g_monitoringOnly) {
      if (RecordFile) {
        writeInProgress = true;
        int bytesWritten = RecordFile->Write((uint8_t *)(recordBuffer + offset),
                                             RECORD_BUFFER_SIZE, 1);
        // sync immediately after writing the buffer for consistent if not
        // fastest perf
        RecordFile->Sync();
        writeInProgress = false;
        if (bytesWritten != RECORD_BUFFER_SIZE) {
          Trace::Error("write failed\r\n");
          // for now just give up and error out
          return;
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
