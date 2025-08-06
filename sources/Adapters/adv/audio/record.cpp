#include "record.h"
#include "Application/Instruments/WavHeaderWriter.h"
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
// Tracking which half is ready
static volatile bool isHalfBuffer = false;
static volatile bool thresholdOK = false;
static bool first_pass = true;
static uint32_t start = 0;
static uint32_t totalSamplesWritten = 0;
static uint32_t recordDuration_ = MAX_INT32;

SemaphoreHandle_t g_recordingFinishedSemaphore = NULL;

bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds) {
  thresholdOK = false;
  first_pass = true;
  g_recordingFinishedSemaphore = xSemaphoreCreateBinary();

  // TODO 0 milliseconds indicates unlimited duration recording
  if (milliseconds != 0) {
    recordDuration_ = milliseconds;
  } else {
    recordDuration_ = MAX_INT32;
  }

  // Create or truncate the file using FileSystem
  RecordFile = FileSystem::GetInstance()->Open(filename, "w");
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

  // Reset sample counter
  totalSamplesWritten = 0;

  // Setup the codec
  tlv320_enable_linein();

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
        RecordFile->Close();
        RecordFile = nullptr;
        // Signal that all file operations are complete.
        xSemaphoreGive(g_recordingFinishedSemaphore);
      }

      Player::GetInstance()->StopRecordStreaming();
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
    if (RecordFile) {
      writeInProgress = true;
      int bytesWritten = RecordFile->Write((uint8_t *)(recordBuffer + offset),
                                           RECORD_BUFFER_SIZE, 1);
      // sync immediately after writing the buffer for consistent if not fastest
      // perf
      RecordFile->Sync();
      writeInProgress = false;
      if (bytesWritten != RECORD_BUFFER_SIZE) {
        Trace::Error("write failed\r\n");
        // for now just give up and error out
        return;
      } else {
        // Total samples written totalSamplesWritten/4 for stereo 16bit samples
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
      Trace::Error("RecordFile is null\r\n");
      return;
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
