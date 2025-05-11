#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <sys/eventfd.h>
#include "moodycamel/concurrentqueue.h"
#include "event_bus.h"

constexpr int EVMESSAGE_TYPE_HEARTBEAT_TIMEOUT = 1;
constexpr int EVMESSAGE_TYPE_SERVER_PUSH = 2;

class EVMessage {
public:
    int fd;
    int type;
    std::any data;
    EVMessage(int fd, int type, std::any data): fd(fd), type(type), data(data) {} 
};

class EventHandler {
private:
    // 当向队列中添加消息时，写这个描述符，用于通知epoll
    int mEventfd;

    // 主线程监听eventfd, 每次触发eventfd可读，就去读这个队列
    moodycamel::ConcurrentQueue<std::shared_ptr<EVMessage>> mConcurrentQueue;

    EventHandler() {}
    ~EventHandler() {}

public:
    static EventHandler& getInstance() {
        static EventHandler m_pusher;
        return m_pusher;
    }

    EventHandler(const EventHandler& p) = delete;
    EventHandler& operator=(const EventHandler& p) = delete;
    EventHandler(EventHandler&&) = delete; 
    EventHandler& operator=(EventHandler&&) = delete;      

    int init();
    int getEventfd() const;
    moodycamel::ConcurrentQueue<std::shared_ptr<EVMessage>>& getConcurrentQueue();
    void cleanEventfd();

    void post(std::shared_ptr<EVMessage> msg);
};


#endif // EVENT_HANDLER_H