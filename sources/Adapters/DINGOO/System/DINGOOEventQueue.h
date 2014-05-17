#ifndef _DINGOO_EVENT_QUEUE_H_
#define _DINGOO_EVENT_QUEUE_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/T_Stack.h"

enum DINGOOEventType {
     DINGOOET_REDRAW
} ;

class DINGOOEvent {
public:
      DINGOOEventType type_;
} ;

class DINGOOEventQueue: public T_Singleton<DINGOOEventQueue>,public T_Stack<DINGOOEvent> {
public:
      DINGOOEventQueue() ;
} ;

#endif
