#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ThreadPool.h"

#define MAX_CLIENT_SIZE 100
#define MAX_BUF_SIZE 512

class Client {
public:
    int fd;
    struct sockaddr_in clientaddr;
    
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