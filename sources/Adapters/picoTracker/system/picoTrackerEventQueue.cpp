#include "picoTrackerEventQueue.h"

picoTrackerEventQueue::picoTrackerEventQueue(){};
void picoTrackerEventQueue::push(picoTrackerEvent event) {
  if (std::find(queue_.begin(), queue_.end(), event) == queue_.end()) {
    queue_.push_back(event);
  }
};

void picoTrackerEventQueue::pop_into(picoTrackerEvent &event) {
  event.type_ = queue_.front().type_;
  queue_.pop_front();
};

bool picoTrackerEventQueue::empty() { return queue_.empty(); }
