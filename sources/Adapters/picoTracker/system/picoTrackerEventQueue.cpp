#include "picoTrackerEventQueue.h"

picoTrackerEventQueue::picoTrackerEventQueue()
    : T_Stack<picoTrackerEvent>(true) {}
