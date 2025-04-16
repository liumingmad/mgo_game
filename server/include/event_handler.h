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
    static constexpr const char* EVENT_COUNTING_POINTS = "EVENT_COUNTING_POINTS";

    // 5.认输 user
    static constexpr const char* EVENT_DEFEAT = "EVENT_DEFEAT";

    // 6.有人进入房间 user
    static constexpr const char* EVENT_GUEST_ENTER_ROOM = "EVENT_GUEST_ENTER_ROOM";

    // 7.有人离开房间 user
    static constexpr const char* EVENT_GUEST_LEAVE_ROOM = "EVENT_GUEST_LEAVE_ROOM";

    // 8.有人发言 user
    static constexpr const char* EVENT_CHAT = "EVENT_CHAT";

    // 8.心跳超时未到达
    static constexpr const char* EVENT_HEART_DISMISS = "EVENT_HEART_DISMISS";

    // 9.落子超时 timer
    static constexpr const char* EVENT_MOVE_TIMEOUT = "EVENT_MOVE_TIMEOUT";

    // 10.离线超时 timer
    static constexpr const char* EVENT_OFFLINE_TIMEOUT = "EVENT_OFFLINE_TIMEOUT";

    // EventHandler(){}
    // ~EventHandler(){}

    void init();
};


#endif // EVENT_HANDLER_H