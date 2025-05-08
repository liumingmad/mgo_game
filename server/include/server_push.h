#ifndef SERVER_PUSH_H
#define SERVER_PUSH_H

#include <string>
#include <map>
#include <atomic>
#include "push_message.h"
#include "wrap.h"
#include "message.h"


// 服务器推送的序列号
static std::atomic<long> g_server_push_serial_number{0};


// 1. 如果遇到掉线的状况，可能客户端没收到
// 推送完，先进入pending队列，如果客户端回复got it，则从队列删除
// 如果客户端没回复got it，超时重发一次。
class ServerPusher {
private:
    std::map<std::string, PushMessage> m_pending_map;

    ServerPusher() {}
    ~ServerPusher() {}

public:
    static ServerPusher& getInstance() {
        static ServerPusher m_pusher;
        return m_pusher;
    }

    ServerPusher(const ServerPusher& p) = delete;
    ServerPusher& operator=(const ServerPusher& p) = delete;

    int server_push(std::string uid, std::shared_ptr<PushMessage> message);
};

int writeResponse(std::shared_ptr<Message> msg, const Response response);


#endif // SERVER_PUSH_H