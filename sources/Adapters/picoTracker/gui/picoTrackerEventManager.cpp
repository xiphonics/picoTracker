#include "picoTrackerEventManager.h"
#include "Adapters/picoTracker/midi/picoTrackerMidiService.h"
#include "Adapters/picoTracker/system/input.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Application.h"
#include "Application/Model/Config.h"
#include "Services/Midi/MidiService.h"
#include "picoTrackerGUIWindowImp.h"
#include "usb_utils.h"

#ifdef USB_REMOTE_UI
#include "picoRemoteUI.h"
#endif

// Key debounce time in milliseconds. No state changes for this amount of time
// means we accept the new key state.
#define KEY_DEBOUNCE_TIME 10

bool picoTrackerEventManager::finished_ = false;
bool picoTrackerEventManager::redrawing_ = false;
uint16_t picoTrackerEventManager::buttonMask_ = 0;

bool picoTrackerEventManager::isRepeating_ = false;
unsigned long picoTrackerEventManager::time_ = 0;
unsigned int picoTrackerEventManager::keyRepeat_ = 25;
unsigned int picoTrackerEventManager::keyDelay_ = 500;
unsigned int picoTrackerEventManager::keyKill_ = 5;

unsigned int picoTrackerEventManager::lastDebounceTime_ = 0;
uint16_t picoTrackerEventManager::debounceMask_ = 0;

repeating_timer_t picoTrackerEventManager::timer_ = repeating_timer_t();
SerialDebugUI picoTrackerEventManager::serialDebugUI_ = SerialDebugUI();

uint16_t gTime_ = 0;

picoTrackerEventQueue *queue;

#ifdef SERIAL_REPL
#define INPUT_BUFFER_SIZE 80
char inBuffer[INPUT_BUFFER_SIZE];
#endif

// timer callback at a rate of 1kHz (from a 1ms hardware interrupt timer)
bool timerHandler(repeating_timer_t *rt) {
  queue = picoTrackerEventQueue::GetInstance();
  gTime_++;

  // send a clock (PICO_CLOCK) with the current tick value
  if (gTime_ % PICO_CLOCK_INTERVAL == 0) {
    queue->push(picoTrackerEvent(PICO_CLOCK));
  }
  return true;
}

int readFromUSBCDC(char *buf, int len) {
  int rc = PICO_ERROR_NO_DATA;
  if (tud_cdc_available()) {
    int count = (int)tud_cdc_read(buf, (uint32_t)len);
    rc = count ? count : PICO_ERROR_NO_DATA;
  }
  return rc;
}

picoTrackerEventManager::picoTrackerEventManager() {}

picoTrackerEventManager::~picoTrackerEventManager() {}

bool picoTrackerEventManager::Init() {
  EventManager::Init();

  keyboardCS_ = new KeyboardControllerSource("keyboard");

  // setup a repeating timer for 1ms ticks
  add_repeating_timer_ms(1, timerHandler, NULL, &timer_);
  return true;
}

int picoTrackerEventManager::MainLoop() {
  queue = picoTrackerEventQueue::GetInstance();
  int loops = 0;
  int events = 0;
#ifdef SDIO_BENCH
  // Perform a benchmark of SD card on startup
  sd_bench();
#endif
  MidiService *midiService = MidiService::GetInstance();
  while (!finished_) {
    loops++;

    // process usb interrupts, should this be done somewhere else??
    handleUSBInterrupts();

    // Poll MIDI service to process any pending MIDI messages
    if (midiService) {
      picoTrackerMidiService *ptMidiService =
          (picoTrackerMidiService *)midiService;
      if (ptMidiService) {
        ptMidiService->poll();
      }
    }

    ProcessInputEvent();
    if (!queue->empty()) {
      picoTrackerEvent event(picoTrackerEventType::LAST);
      queue->pop_into(event);
      events++;
      redrawing_ = true;
      picoTrackerGUIWindowImp::ProcessEvent(event);
      redrawing_ = false;
    }
#ifdef PICOSTATS
    if (loops == 100000) {
      Trace::Debug("Usage %.1f% CPU", ((float)events / loops) * 100);
      events = 0;
      loops = 0;
      //      measure_freqs();
      measure_free_mem();
    }
#endif
  }
  // TODO: HW Shutdown
  return 0;
}

void picoTrackerEventManager::PostQuitMessage() {
  // Trace:Log("EVENT", "quit");
  finished_ = true;
}

int picoTrackerEventManager::GetKeyCode(const char *name) { return -1; }

void picoTrackerEventManager::ProcessInputEvent() {
  uint16_t newMask, sendMask;

  if (redrawing_)
    return;
  bool gotEvent = false;

  // Get current mask
  newMask = scanKeys();

  unsigned long now = gTime_;

  if (newMask != debounceMask_) {
    // Key state changed. We begin or continue debouncing.
    debounceMask_ = newMask;
    lastDebounceTime_ = now;
    return;
  } else {
    // Keys have not changed since the last scan. But we cannot
    // continue unless they have not changed for at least KEY_DEBOUNCE_TIME ms
    unsigned long settleTime = now - lastDebounceTime_;
    if (settleTime < KEY_DEBOUNCE_TIME) {
      return;
    }
  }

  // compute mask to send
  sendMask = (newMask ^ buttonMask_) |
             (newMask & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN));

  // see if we're repeating
  if (newMask == buttonMask_) {
    if ((isRepeating_) && ((now - time_) > keyRepeat_)) {
      gotEvent = (sendMask != 0);
    }
    if ((!isRepeating_) && ((now - time_) > keyDelay_)) {
      gotEvent = (sendMask != 0);
      if (gotEvent)
        isRepeating_ = true;
    }
  } else {
    if ((now - time_) > keyKill_) {
      gotEvent = (sendMask != 0);
      if (gotEvent)
        isRepeating_ = false;
    }
  }
  if (gotEvent) {
    time_ = gTime_; // Get time here so delay is independant of processing speed

    //                Trace::Debug("Pe") ;
    picoTrackerGUIWindowImp::ProcessButtonChange(sendMask, newMask);
    buttonMask_ = newMask;
    //            Trace::Debug("%d: mask=%x",gTime_,sendMask) ;
    //                Trace::Debug("~Pe") ;
  }

#ifdef SERIAL_REPL
  serialDebugUI_.readSerialIn(inBuffer, INPUT_BUFFER_SIZE);
#endif
#ifdef USB_REMOTE_UI
  char inBuffer[16];
  auto readbytes = readFromUSBCDC(inBuffer, 16);
  if (readbytes > 0) {
    Trace::Debug("Read %d bytes from USB CDC", readbytes);
    if (inBuffer[0] != REMOTE_INPUT_CMD_MARKER) {
      Trace::Debug("Invalid input command marker %d", inBuffer[0]);
      return;
    }
    switch (inBuffer[1]) {
    case FULL_REFRESH_CMD:
      Trace::Debug("Full refresh requested!");
      queue = picoTrackerEventQueue::GetInstance();
      queue->push(picoTrackerEvent(PICO_REDRAW));
      break;

    default:
      break;
    }
  }
#endif
}
