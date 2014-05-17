#ifndef _GP2X_EVENT_QUEUE_H_
#define _GP2X_EVENT_QUEUE_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/T_Stack.h"

enum GP2XEventType {
     GP2XET_REDRAW
} ;

class GP2XEvent {
public:
      GP2XEventType type_;
} ;

class GP2XEventQueue: public T_Singleton<GP2XEventQueue>,public T_Stack<GP2XEvent> {
public:
      GP2XEventQueue() ;
} ;

#endif
