#include "advSystem.h"
#include "Adapters/adv/audio/advAudio.h"
#include "Adapters/adv/filesystem/picoFileSystem.h"
#include "Adapters/adv/gui/GUIFactory.h"
#include "Adapters/adv/midi/advMidiService.h"
#include "Adapters/adv/system/advSamplePool.h"
#include "Adapters/adv/timer/advTimer.h"
#include "Application/Commands/NodeList.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "BatteryGauge.h"
#include "critical_error_message.h"
#include "i2c.h"
#include "input.h"
#include "platform.h"
#include "tim.h"
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

EventManager *advSystem::eventManager_ = NULL;
bool advSystem::invert_ = false;
int advSystem::lastBattLevel_ = 100;
unsigned int advSystem::lastBeatCount_ = 0;

int advSystem::MainLoop() {

  eventManager_->InstallMappings();
  return eventManager_->MainLoop();
};

void advSystem::Boot() {

  // Start high resolution timer
  HAL_TIM_Base_Start(&htim2);

  // Install System
  __attribute__((
      section(".DATA_RAM"))) static char systemMemBuf[sizeof(advSystem)];
  System::Install(new (systemMemBuf) advSystem());

  // Install GUI Factory
  __attribute__((
      section(".DATA_RAM"))) static char guiMemBuf[sizeof(GUIFactory)];
  I_GUIWindowFactory::Install(new (guiMemBuf) GUIFactory());

  // Install Timers
  __attribute__((
      section(".DATA_RAM"))) static char timerMemBuf[sizeof(advTimerService)];
  TimerService::GetInstance()->Install(new (timerMemBuf) advTimerService());

  // Install FileSystem
  __attribute__((
      section(".DATA_RAM"))) static char fsMemBuf[sizeof(picoFileSystem)];
  FileSystem::Install(new (fsMemBuf) picoFileSystem());

  // First check for SDCard
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir("/")) {
    Trace::Log("PICOTRACKERSYSTEM", "SDCARD MISSING!!\n");
    critical_error_message("SDCARD MISSING", 0x01);
  }

  // Install MIDI
  // **NOTE**: MIDI install MUST happen before Audio install because it triggers
  // reading config file and config file needs to have MidiService already
  // installed in order to apply midi settings read from the config file
  __attribute__((
      section(".DATA_RAM"))) static char midiMemBuf[sizeof(advMidiService)];
  MidiService::Install(new (midiMemBuf) advMidiService());

  // Install Sound
  AudioSettings hint;
  hint.bufferSize_ = 1024;
  hint.preBufferCount_ = 8;
  __attribute__((
      section(".DATA_RAM"))) static char audioMemBuf[sizeof(advAudio)];
  Audio::Install(new (audioMemBuf) advAudio(hint));

  // Install SamplePool
  static char samplePoolMemBuf[sizeof(advSamplePool)];
  SamplePool::Install(new (samplePoolMemBuf) advSamplePool());

  // Configure the battery fuel gauge - will only update if ITPOR bit is set
  configureBatteryGauge();

  eventManager_ = I_GUIWindowFactory::GetInstance()->GetEventManager();
  eventManager_->Init();
};

void advSystem::Shutdown() { delete Audio::GetInstance(); };

static int secbase;

unsigned long advSystem::GetClock() {
  struct timeval tp;

  gettimeofday(&tp, NULL);
  if (!secbase) {
    secbase = tp.tv_sec;
    return long(tp.tv_usec / 1000.0);
  }
  return long((tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000.0);
}

void advSystem::GetBatteryState(BatteryState &state) {
  state.percentage = getBatterySOC();
  state.voltage_mv = getBatteryVoltage();
  state.temperature_c = getBatteryTemperature();

  // TODO: in future just get that actual charging state from the chargerIC
  int16_t current = getBatteryCurrent();
  if (current != CURRENT_READ_ERROR) {
    if (current > 50) {
      state.charging = true;
    } else {
      state.charging = false;
    }
  } else {
    // default to false on error
    state.charging = false;
  }
}

void advSystem::SetDisplayBrightness(unsigned char value) {
  platform_brightness(value);
}

void advSystem::Sleep(int millisec) {
  //	if (millisec>0)
  //		assert(0) ;
}

void *advSystem::Malloc(unsigned size) {
  void *ptr = malloc(size);
  return ptr;
}

void advSystem::Free(void *ptr) { free(ptr); }

void advSystem::Memset(void *addr, char val, int size) {
  memset(addr, val, size);
};

void *advSystem::Memcpy(void *s1, const void *s2, int n) {
  return memcpy(s1, s2, n);
}

void advSystem::PostQuitMessage() { eventManager_->PostQuitMessage(); }

unsigned int advSystem::GetMemoryUsage() {
  struct mallinfo m = mallinfo();
  return m.uordblks;
}

void advSystem::PowerDown() {
  // Ship mode
  uint8_t value = 0x64;
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
      &hi2c4, 0x6b << 1, 0x07, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }

  setCharging();

  HAL_GPIO_DeInit(POWER_GPIO_Port, POWER_Pin);
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // always clear before enabling again
  PWREx_WakeupPinTypeDef sPinParams = {
      .WakeUpPin = PWR_WAKEUP_PIN2,
      .PinPolarity = PWR_PIN_POLARITY_LOW, // Rising edge triggers wakeup
      .PinPull = PWR_PIN_NO_PULL // Pulldown to ensure low level before press
  };
  HAL_PWREx_EnableWakeUpPin(&sPinParams);
  HAL_PWR_EnterSTANDBYMode();
}

void advSystem::setCharging(void) {
  uint8_t value = 0x1a;
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
      &hi2c4, 0x6b << 1, 0x01, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }
  HAL_GPIO_WritePin(CHARGER_OTG_GPIO_Port, CHARGER_OTG_Pin, GPIO_PIN_RESET);
}
