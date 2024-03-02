#include "picoTrackerEventManager.h"
#include "Adapters/picoTracker/system/input.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Application.h"
#include "picoTrackerGUIWindowImp.h"

bool picoTrackerEventManager::finished_ = false;
bool picoTrackerEventManager::redrawing_ = false;
uint16_t picoTrackerEventManager::buttonMask_ = 0;

bool picoTrackerEventManager::isRepeating_ = false;
unsigned long picoTrackerEventManager::time_ = 0;
unsigned int picoTrackerEventManager::keyRepeat_ = 25;
unsigned int picoTrackerEventManager::keyDelay_ = 500;
unsigned int picoTrackerEventManager::keyKill_ = 5;
repeating_timer_t picoTrackerEventManager::timer_ = repeating_timer_t();

uint16_t gTime_ = 0;

picoTrackerEventQueue *queue;

bool timerHandler(repeating_timer_t *rt) {
  gTime_++;
  if (gTime_ % 1000 == 0) {
    //    auto event = new picoTrackerEvent();
    //    event->type_ = PICO_CLOCK;
    if (!queue->full()) {
      queue->push(picoTrackerEvent(PICO_CLOCK));
    }
  }
  return true;
}

picoTrackerEventManager::picoTrackerEventManager() {}

picoTrackerEventManager::~picoTrackerEventManager() {}

bool picoTrackerEventManager::Init() {
  EventManager::Init();

  keyboardCS_ = new KeyboardControllerSource("keyboard");

  // TODO: fix this, there is a timer service that should be used. Also all of
  // this keyRepeat logic is already implemented in the eventdispatcher
  // Application/Commands/EventDispatcher.cpp
  add_repeating_timer_ms(1, timerHandler, NULL, &timer_);
  return true;
}

int picoTrackerEventManager::MainLoop() {
  queue = picoTrackerEventQueue::GetInstance();
  int loops = 0;
  int events = 0;
  while (!finished_) {
    loops++;
    ProcessInputEvent();
    if (!queue->empty()) {
      picoTrackerEvent event(PICO_NONE);
      queue->pop_into(event);
      events++;
      redrawing_ = true;
      picoTrackerGUIWindowImp::ProcessEvent(event);
      queue->clear(); // Avoid duplicates redraw
      redrawing_ = false;
    }
#ifdef PICOSTATS
    if (loops == 100000) {
      printf("Usage %.1f% CPU\n", ((float)events / loops) * 100);
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

  // compute mask to send
  sendMask = (newMask ^ buttonMask_) |
             (newMask & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN));
  unsigned long now = gTime_;
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
}
