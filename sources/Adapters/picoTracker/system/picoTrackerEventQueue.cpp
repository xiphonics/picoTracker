#include "picoTrackerEventQueue.h"

picoTrackerEventQueue::picoTrackerEventQueue(){};
void picoTrackerEventQueue::push(picoTrackerEvent event) {
  if (!queued_.contains(event.type_)) {
    queue_.push(event);
    queued_.insert(event.type_);
  }
};

void picoTrackerEventQueue::pop_into(picoTrackerEvent &event) {
  queued_.erase(event.type_);
  queue_.pop_into(event);
};

bool picoTrackerEventQueue::empty() { return queue_.empty(); }
