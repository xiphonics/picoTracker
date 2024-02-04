#ifndef _PICOTRACKEREVENTQUEUE_H_
#define _PICOTRACKEREVENTQUEUE_H_

#include "../../../Externals/etl/include/etl/stack.h"
#include "Foundation/T_Singleton.h"
#include "Foundation/T_Stack.h"

enum picoTrackerEventType {
  //  PICO_KEYDOWN,
  //  PICO_KEYUP,
  //  PICO_QUIT,
  PICO_REDRAW,
  PICO_CLOCK,
  //  PICO_USEREVENT
};

class picoTrackerEvent {
public:
  picoTrackerEventType type_;
};

class picoTrackerEventQueue : public T_Singleton<picoTrackerEventQueue>,
                              public etl::stack<picoTrackerEvent, 5> {
public:
  picoTrackerEventQueue();
};

#endif
