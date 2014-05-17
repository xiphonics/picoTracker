#ifndef _CAANOO_EVENT_QUEUE_H_
#define _CAANOO_EVENT_QUEUE_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/T_Stack.h"

enum CAANOOEventType {
     CAANOOET_REDRAW
} ;

class CAANOOEvent {
public:
      CAANOOEventType type_;
} ;

class CAANOOEventQueue: public T_Singleton<CAANOOEventQueue>,public T_Stack<CAANOOEvent> {
public:
      CAANOOEventQueue() ;
} ;

#endif
