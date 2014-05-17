#ifndef _NDS_EVENT_QUEUE_H_
#define _NDS_EVENT_QUEUE_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/T_Stack.h"

enum NDSEventType {
     NDSET_REDRAW,
     NDSET_ESC
} ;

class NDSEvent {
public:
      NDSEventType type_;
} ;

class NDSEventQueue: public T_Singleton<NDSEventQueue>,public T_Stack<NDSEvent> {
} ;

#endif
