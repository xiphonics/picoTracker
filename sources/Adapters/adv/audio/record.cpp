#include "record.h"
#include "System/Console/Trace.h"
#include "sai.h"
#include "sd_diskio.h"
#include "tlv320aic3204.h"
#include <cstdio>
#include <cstring>

// StackType_t RecordStack[1024];
// StaticTask_t RecordTCB;

static FIL RecordFile;

uint8_t *activeBuffer;
uint8_t *writeBuffer;

__attribute__((section(".FRAMEBUFFER"))) __attribute__((aligned(32)))
uint8_t bufferA[RECORD_BUFFER_SIZE];
__attribute__((section(".FRAMEBUFFER"))) __attribute__((aligned(32)))
uint8_t bufferB[RECORD_BUFFER_SIZE];

// SemaphoreHandle_t record_semaphore = NULL;
// StaticSemaphore_t record_semaphoreBuffer;
TaskHandle_t RecordHandle = NULL;

volatile bool bufferA_ready = false;
volatile bool bufferB_ready = false;
volatile bool using_bufferA = true;

static volatile bool recordingActive = false;

bool StartRecording(const char *filename) {
  // Create or truncate the file
  auto fs_status =
      f_open(&RecordFile, filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
  if (fs_status != FR_OK)
    return false;

  // If file already exists, truncate it
  f_truncate(&RecordFile);

  // Setup the codec
  tlv320_enable_linein();

  // Clear / init buffers
  memset(bufferA, 0, RECORD_BUFFER_SIZE);
  memset(bufferB, 0, RECORD_BUFFER_SIZE);
  activeBuffer = bufferA;
  writeBuffer = bufferB;

  // Start first DMA transfer
  HAL_SAI_Receive_DMA(&hsai_BlockB1, activeBuffer, RECORD_BUFFER_SIZE / 2);

  // Signal task to start recording
  recordingActive = true;

  // Resume or create task if needed
  vTaskResume(RecordHandle);

  return true;
}

void Record(void *) {
  //  record_semaphore = xSemaphoreCreateBinaryStatic(&record_semaphoreBuffer);
  UINT bw;
  for (;;) {
    // Interrupt return
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
      // DMA transmit finished, dump to file
      // setup next receive
      if (HAL_SAI_Receive_DMA(&hsai_BlockB1, activeBuffer,
                              RECORD_BUFFER_SIZE) != HAL_OK) {
        Trace::Error("RECORD", "Issue recording");
      }

      f_write(&RecordFile, writeBuffer, RECORD_BUFFER_SIZE, &bw);
      if (bw != RECORD_BUFFER_SIZE) {
        Trace::Error("write failed\r\n");
      }

      Trace::Debug("Record event");
    }
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /*
  if (using_bufferA) {
    bufferA_ready = true;
    HAL_SAI_Receive_DMA(hsai, bufferB, AUDIO_BUFFER_SIZE);
    using_bufferA = false;
  } else {
    bufferB_ready = true;
    HAL_SAI_Receive_DMA(hsai, bufferA, AUDIO_BUFFER_SIZE);
    using_bufferA = true;
  }
  */
  // swap buffers
  uint8_t *temp = activeBuffer;
  activeBuffer = writeBuffer;
  writeBuffer = temp;

  vTaskNotifyGiveFromISR(RecordHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
