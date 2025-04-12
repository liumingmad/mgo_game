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

#define MAX_CLIENT_SIZE 100
constexpr size_t BUF_SIZE = 512;
constexpr size_t RING_BUFFER_SIZE = 128 * 1024;

class Client {
public:
    int fd;
    struct sockaddr_in clientaddr;
    RingBuffer* ringBuffer;
    Core* core;

    Client() {

    }

    ~Client() {

    }
};

static std::map<int, Client> clientMap;

class Server {
private:
    ThreadPool m_pool;

    // 全局定时器，每2s触发一次，用于所有正在进行的对局，触发超时认负
    Timer m_timer;

public:
    int init();
    int run(int port);
    int handle_request(int fd);
    
    int add_client(int fd, struct sockaddr_in addr);
    int remove_client(int fd);
    int shutdown();

};




#endif // SERVER_H