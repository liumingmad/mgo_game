#ifndef ROOM_UPDATE_SUBSCRIBER_H
#define ROOM_UPDATE_SUBSCRIBER_H

#include "event_bus.h"

class EventHandler {
public:
    EventHandler(){}
    ~EventHandler(){}

    void init(AsyncEventBus& bus);
};


#endif // ROOM_UPDATE_SUBSCRIBER_H