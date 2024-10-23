#include "EventDispatcher.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"

int EventDispatcher::keyRepeat_ = 30;
int EventDispatcher::keyDelay_ = 500;

EventDispatcher::EventDispatcher() {
  window_ = 0;
  eventMask_ = 0;

  // Read config file key repeat

  Config *config = Config::GetInstance();
  keyDelay_ = config->GetValue("KEYDELAY");
  keyRepeat_ = config->GetValue("KEYREPEAT");

  repeatMask_ = 0;
  repeatMask_ |= (1 << EPBT_LEFT);
  repeatMask_ |= (1 << EPBT_RIGHT);
  repeatMask_ |= (1 << EPBT_UP);
  repeatMask_ |= (1 << EPBT_DOWN);

  timer_ = TimerService::GetInstance()->CreateTimer();
  timer_->AddObserver(*this);
};

EventDispatcher::~EventDispatcher() {
  timer_->RemoveObserver(*this);
  SAFE_DELETE(timer_);
};

void EventDispatcher::Execute(FourCC id, float value) {

  if (window_) {
    GUIEventPadButtonType mapping = EPBT_INVALID;
    switch (id) {
    case FourCC::TrigEventEnter:
      mapping = EPBT_A;
      break;
    case FourCC::TrigEventEdit:
      mapping = EPBT_B;
      break;
    case FourCC::TrigEventLeft:
      mapping = EPBT_LEFT;
      break;
    case FourCC::TrigEventRight:
      mapping = EPBT_RIGHT;
      break;
    case FourCC::TrigEventUp:
      mapping = EPBT_UP;
      break;
    case FourCC::TrigEventDown:
      mapping = EPBT_DOWN;
      break;
    case FourCC::TrigEventAlt:
      mapping = EPBT_L;
      break;
    case FourCC::TrigEventNav:
      mapping = EPBT_R;
      break;
    case FourCC::TrigEventPlay:
      mapping = EPBT_START;
      break;
      //	EPBT_SELECT
    }

    // Compute mask and repeat if needed

    if (value > 0.5) {
      eventMask_ |= (1 << mapping);
    } else {
      eventMask_ ^= (1 << mapping);
    }

    // Dispatch event to window

    unsigned long now = System::GetInstance()->GetClock();
    GUIEventType type = (value > 0.5) ? ET_PADBUTTONDOWN : ET_PADBUTTONUP;
    GUIEvent event(mapping, type, now, 0, 0, 0);
    window_->DispatchEvent(event);

    if (eventMask_ & repeatMask_) {
      timer_->SetPeriod(float(keyDelay_));
      timer_->Start();
    } else {
      timer_->Stop();
    }
  };
};

void EventDispatcher::SetWindow(GUIWindow *window) { window_ = window; };

unsigned int EventDispatcher::OnTimerTick() {

  unsigned sendMask = (eventMask_ & repeatMask_);
  unsigned long now = System::GetInstance()->GetClock();

  if (sendMask) {
    int current = 0;
    while (sendMask) {
      if (sendMask & 1) {
        GUIEvent event(current, ET_PADBUTTONDOWN, now, 0, 0, 0);
        window_->DispatchEvent(event);
      }
      sendMask >>= 1;
      current++;
    }
    return keyRepeat_;
  }
  return 0;
};

void EventDispatcher::Update(Observable &o, I_ObservableData *d) {
  unsigned int tick = OnTimerTick();
  if (tick) {
    timer_->SetPeriod(float(tick));
  } else {
    timer_->Stop();
  };
};
