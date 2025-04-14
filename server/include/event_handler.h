#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "event_bus.h"

class EventHandler {
public:
    // 1.落子 user
    static constexpr const char* EVENT_MOVE = "EVENT_MOVE";

    // 2.上线 epool
    static constexpr const char* EVENT_ONLINE = "EVENT_ONLINE";

    // 3.离线 epool
    static constexpr const char* EVENT_OFFLINE = "EVENT_OFFLINE";

    // 4.数子 user
    // 5.认输 user
    // 6.有人进入房间 user
    // 7.有人发言 user
    // 8.无心跳
    // 9.落子超时 timer
    // 10.离线超时 timer

    // EventHandler(){}
    // ~EventHandler(){}

    void init();
};


#endif // EVENT_HANDLER_H