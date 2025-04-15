#ifndef _PICOTRACKEREVENTQUEUE_H_
#define _PICOTRACKEREVENTQUEUE_H_

#include "Externals/etl/include/etl/queue_spsc_atomic.h"
#include "Foundation/T_Singleton.h"

enum picoTrackerEventType { PICO_REDRAW, PICO_FLUSH, PICO_CLOCK, LAST };

class picoTrackerEvent {
public:
  picoTrackerEvent(picoTrackerEventType type) : type_(type) {}
  picoTrackerEventType type_;
};

inline bool operator==(const picoTrackerEvent &lhs,
                       const picoTrackerEvent &rhs) {
  return lhs.type_ == rhs.type_;
};

class picoTrackerEventQueue : public T_Singleton<picoTrackerEventQueue> {
public:
  picoTrackerEventQueue();
  void push(picoTrackerEvent event);
  void pop_into(picoTrackerEvent &event);
  bool empty();

private:
  static const size_t EVENT_QUEUE_SIZE = 256;
  etl::queue_spsc_atomic<picoTrackerEvent, EVENT_QUEUE_SIZE> queue_;
};

#endif
