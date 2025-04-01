#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ThreadPool.h"
#include "Log.h"
#include "ring_buffer.h"

#define MAX_CLIENT_SIZE 100
constexpr size_t RING_BUFFER_SIZE = 128 * 1024;

class Client {
public:
    int fd;
    struct sockaddr_in clientaddr;
    RingBuffer* ringBuffer;    
};


class Server {
private:
    std::map<int, Client> clientMap;
    ThreadPool pool;

public:
    int init();
    int run(int port);
    int handle_request(int fd);
    
    int add_client(int fd, struct sockaddr_in addr);
    int remove_client(int fd);
    int shutdown();
};




#endif // SERVER_H