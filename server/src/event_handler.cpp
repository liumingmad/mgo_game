#include "event_handler.h"

void onEventMove(const std::any move) {
}

void onEventOnline(const std::any user_id) {
    if (user_id.type() == typeid(std::string)) {
        std::cout << std::any_cast<std::string>(user_id) << std::endl;
    }
}

void onEventOffline(const std::any user_id) {
    if (user_id.type() == typeid(std::string)) {
        std::cout << std::any_cast<std::string>(user_id) << std::endl;
    }
}



// 客户端应该把room当作一个播放器
// 当room状态发生变化时，把room的更新发送到房间内所有人

// 一些触发room更新的操作:
// 1.落子 user
// 2.上线 epool
// 3.离线 epool
// 4.数子 user
// 5.认输 user
// 6.有人进入房间 user
// 7.有人发言 user
// 8.无心跳
// 9.落子超时 timer
// 10.离线超时 timer

// 启动一个新线程，消费上述事件，订阅事件总线，上述10种情况是发布端，收到事件后，把room更新的消息，发送给room内所有人
void EventHandler::init(AsyncEventBus& bus) {
    bus.subscribe("event_online", onEventOnline);
    bus.subscribe("event_offline", onEventOffline);
    bus.subscribe("event_move", onEventMove);
}