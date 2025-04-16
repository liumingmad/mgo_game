#include "event_handler.h"


void onEventTMP(const std::string move) {
}

void onEventOnline(const std::string user_id) {
    std::cout << "onEventOnline() "  << user_id << std::endl;
}

void onEventOffline(const std::string user_id) {
    std::cout << "onEventOffline() " << user_id << std::endl;
}

void onEventMove(const std::string move) {
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
void EventHandler::init() {
    AsyncEventBus& bus = AsyncEventBus::getInstance();
    bus.subscribe<std::string>(EventHandler::EVENT_ONLINE, onEventOnline);
    bus.subscribe<std::string>(EventHandler::EVENT_OFFLINE, onEventOffline);
    bus.subscribe<std::string>(EventHandler::EVENT_MOVE, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_DEFEAT, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_GUEST_ENTER_ROOM, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_GUEST_LEAVE_ROOM, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_CHAT, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_HEART_DISMISS, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_MOVE_TIMEOUT, onEventTMP);
    bus.subscribe<std::string>(EventHandler::EVENT_OFFLINE_TIMEOUT, onEventTMP);
}