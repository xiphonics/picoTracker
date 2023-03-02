#ifndef _PICO_EVENT_QUEUE_H_
#define _PICO_EVENT_QUEUE_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/T_Stack.h"

enum PICOEventType {
  //  PICO_KEYDOWN,
  //  PICO_KEYUP,
  PICO_QUIT,
  PICO_REDRAW,
  //  PICO_USEREVENT
};

class PICOEvent {
public:
  PICOEventType type_;
};

class PICOEventQueue : public T_Singleton<PICOEventQueue>,
                       public T_Stack<PICOEvent> {
public:
  PICOEventQueue();
};

#endif
