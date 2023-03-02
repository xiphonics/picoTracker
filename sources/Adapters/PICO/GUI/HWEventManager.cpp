#include "HWEventManager.h"
#include "Adapters/PICO/System/input.h"
#include "Adapters/PICO/Utils/utils.h"
#include "Application/Application.h"
#include "HWGUIWindowImp.h"

bool HWEventManager::finished_ = false;
bool HWEventManager::redrawing_ = false;
bool HWEventManager::escPressed_ = false;
uint16_t HWEventManager::buttonMask_ = 0;

bool HWEventManager::isRepeating_ = false;
unsigned long HWEventManager::time_ = 0;
unsigned int HWEventManager::keyRepeat_ = 25;
unsigned int HWEventManager::keyDelay_ = 500;
unsigned int HWEventManager::keyKill_ = 5;
repeating_timer_t HWEventManager::timer_ = repeating_timer_t();

uint16_t gTime_ = 0;

bool timerHandler(repeating_timer_t *rt) {
  gTime_++;
  return true;
}

HWEventManager::HWEventManager() {}

HWEventManager::~HWEventManager() {}

bool HWEventManager::Init() {
  EventManager::Init();

  keyboardCS_ = new KeyboardControllerSource("keyboard");

  // TODO: fix this, there is a timer service that should be used. Also all of
  // this keyRepeat logic is already implemented in the eventdispatcher
  // Application/Commands/EventDispatcher.cpp
  add_repeating_timer_ms(1, timerHandler, NULL, &timer_);
  return true;
}

int HWEventManager::MainLoop() {
  //	GUIWindow *appWindow=Application::GetInstance()->GetWindow() ;
  //	HWGUIWindowImp *hwWindow=(HWGUIWindowImp *)appWindow->GetImpWindow() ;

  //	hwWindow->ProcessExpose() ;

  PICOEventQueue *queue = PICOEventQueue::GetInstance();
  bool finished = false;
  int loops = 0;
  int events = 0;
  while (!finished_) {
    loops++;
    ProcessInputEvent();
    PICOEvent *event = queue->Pop(true);
    if (event) {
      events++;
      redrawing_ = true;
      HWGUIWindowImp::ProcessEvent(*event);
      delete event;
      queue->Empty(); // Avoid duplicates redraw
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

void HWEventManager::PostQuitMessage() {
  // Trace:Log("EVENT", "quit");
  finished_ = true;
}

int HWEventManager::GetKeyCode(const char *name) { return -1; }

void HWEventManager::ProcessInputEvent() {
  uint16_t newMask, sendMask;

  if (redrawing_)
    return;
  bool gotEvent = false;

  newMask = scanKeys(); // Get current mask
  if ((newMask & KEY_SELECT) && (newMask & KEY_L) && (newMask & KEY_R)) {
    if (!escPressed_) {
      PICOEvent event;
      event.type_ = PICO_QUIT;
      HWGUIWindowImp::ProcessEvent(event);
      escPressed_ = true;
    }
  } else {
    escPressed_ = false;
  };

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
    HWGUIWindowImp::ProcessButtonChange(sendMask, newMask);
    buttonMask_ = newMask;
    //            Trace::Debug("%d: mask=%x",gTime_,sendMask) ;
    //                Trace::Debug("~Pe") ;
  }
}
