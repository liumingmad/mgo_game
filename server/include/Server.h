#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ThreadPool.h"
#include "Log.h"
#include "ring_buffer.h"
#include "core.h"
#include "timer.h"
#include "push_message.h"
#include "server_push.h"
#include "event_handler.h"

#define MAX_CLIENT_SIZE 100
constexpr size_t BUF_SIZE = 512;
constexpr size_t RING_BUFFER_SIZE = 128 * 1024;

class Client
{
public:
    int fd;
    struct sockaddr_in clientaddr;
    std::shared_ptr<RingBuffer> ringBuffer;
    std::shared_ptr<Core> core;
    std::string user_id;

    // 当执行queue中Message的过程中，不能执行队列中下一个
    std::mutex mutex;
    SafeQueue<Message> queue;

    Client() : ringBuffer(std::make_shared<RingBuffer>(RING_BUFFER_SIZE)),
               core(std::make_shared<Core>())
    {
    }

    ~Client() {}

    Client(Client& client) = delete;
    Client& operator=(const Client&) = delete;
    
};

class Server
{
private:
    int m_epfd;
    struct epoll_event m_epoll_event;

    ThreadPool m_pool;

    // 全局定时器，每2s触发一次，用于所有正在进行的对局，触发超时认负
    Timer m_timer;

    EventHandler m_event_handler;

public:
    int init();
    int run(int port);
    int handle_request(std::shared_ptr<Client> client);
    void handle_message(std::shared_ptr<Client> client);
    void heart_timeout(int fd);

    int add_client(int fd, struct sockaddr_in addr);
    int remove_client(int fd);
    int shutdown();
};

#endif // SERVER_H