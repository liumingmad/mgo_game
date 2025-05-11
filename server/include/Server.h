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

#define MAX_CLIENT_SIZE 100
constexpr size_t BUF_SIZE = 512;
constexpr size_t RING_BUFFER_SIZE = 128 * 1024;

extern long global_client_id;

class Client
{
public:
    int id;
    int fd;
    struct sockaddr_in clientaddr;
    // 上次收到客户端的message的时间
    long activeTimestamp;
    std::shared_ptr<Core> core;
    std::string user_id;

    // 当执行queue中Message的过程中，不能执行队列中下一个
    std::mutex mutex;
    SafeQueue<std::shared_ptr<Message>> queue;

    // 读缓冲
    std::shared_ptr<RingBuffer> ringBuffer;
    // 写缓冲，保存每个连接剩余未写完的数据
    std::string pendingWriteBuffer;

    Client() : ringBuffer(std::make_shared<RingBuffer>(RING_BUFFER_SIZE)),
               core(std::make_shared<Core>())
    {
        id = global_client_id++;
    }

    ~Client() {}

    Client(Client& client) = delete;
    Client& operator=(const Client&) = delete;
    
};

class Server
{
private:
    int m_epfd;
    
    // 处理client业务的线程池
    ThreadPool m_pool;

    Timer m_timer;

public:
    int init();
    int run(int port);
    int handle_read(std::shared_ptr<Client> client);
    void handle_message(std::shared_ptr<Client> client);

    int shutdown();
};


int add_client(int fd, struct sockaddr_in addr);
int remove_client(int fd);
void handle_write(int fd, int epfd);
void schedule_write(int fd, const std::string &data, int epfd);
void handleEventfd(int epfd);
void handleListenfd(int fd, int epfd);

#endif // SERVER_H