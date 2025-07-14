#include "record.h"
#include "Application/Player/Player.h"
#include "System/Console/Trace.h"
#include "sai.h"
#include "sd_diskio.h"
#include "tlv320aic3204.h"
#include <cstdio>
#include <cstring>

static FIL RecordFile;

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

bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds) {
  thresholdOK = false;
  first_pass = true;
  // Create or truncate the file
  auto fs_status =
      f_open(&RecordFile, filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
  if (fs_status != FR_OK) {
    Trace::Log("RECORD", "Could not create file");
    return false;
  }

  // If file already exists, truncate it
  f_truncate(&RecordFile);

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

void stopRecording() { recordingActive = false; }
void Record(void *) {
  UINT bw;
  for (;;) {
    if ((xTaskGetTickCount() - start) > 20000) {
      Trace::Log("RECORD", "(%i) stop recording",
                 (xTaskGetTickCount() - start));
      recordingActive = false;
    }

    if (!recordingActive) {
      HAL_SAI_DMAStop(&hsai_BlockB1);
      // sync file and close (if open)
      f_sync(&RecordFile);
      f_close(&RecordFile);
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
    writeInProgress = true;
    // full buffer length as uint8_t (which is half of the uint16_t buffer)
    f_write(&RecordFile, (uint8_t *)(recordBuffer + offset), RECORD_BUFFER_SIZE,
            &bw);
    f_sync(&RecordFile);
    writeInProgress = false;
    if (bw != RECORD_BUFFER_SIZE) {
      Trace::Error("write failed\r\n");
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
