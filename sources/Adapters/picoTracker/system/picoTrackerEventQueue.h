#ifndef _PICOTRACKEREVENTQUEUE_H_
#define _PICOTRACKEREVENTQUEUE_H_

#include "Externals/etl/include/etl/queue.h"
#include "Externals/etl/include/etl/set.h"
#include "Foundation/T_Singleton.h"

enum picoTrackerEventType { PICO_REDRAW, PICO_CLOCK, LAST };

class picoTrackerEvent {
public:
  picoTrackerEvent(picoTrackerEventType type) : type_(type) {}

  picoTrackerEventType type_;
};

class picoTrackerEventQueue : public T_Singleton<picoTrackerEventQueue> {
public:
  picoTrackerEventQueue();
  void push(picoTrackerEvent event);
  void pop_into(picoTrackerEvent &event);
  bool empty();

private:
  etl::queue<picoTrackerEvent, picoTrackerEventType::LAST,
             etl::memory_model::MEMORY_MODEL_SMALL>
      queue_;
  etl::set<picoTrackerEventType, picoTrackerEventType::LAST> queued_;
};

#endif
