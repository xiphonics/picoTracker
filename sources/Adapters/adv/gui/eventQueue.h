#ifndef _EVENT_QUEUE_
#define _EVENT_QUEUE_
#include "FreeRTOS.h"
#include "queue.h"

enum EventType { REDRAW, FLUSH, CLOCK, SD_DET_INSERT, SD_DET_REMOVE, LAST };

class Event {
public:
  Event(EventType type) : type_(type) {}
  EventType type_;
};

#define EVENT_QUEUE_LENGTH 16
#define EVENT_QUEUE_ITEM_SIZE sizeof(Event)
static StaticQueue_t eventQueueBuffer;
extern uint8_t eventQueueStorage;
extern QueueHandle_t eventQueue;
#endif
