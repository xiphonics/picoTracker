#include "advEventQueue.h"

advEventQueue::advEventQueue(){};
void advEventQueue::push(advEvent event) {
  if (std::find(queue_.begin(), queue_.end(), event) == queue_.end()) {
    queue_.push_back(event);
  }
};

void advEventQueue::pop_into(advEvent &event) {
  event.type_ = queue_.front().type_;
  queue_.pop_front();
};

bool advEventQueue::empty() { return queue_.empty(); }
