#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "wrap.h"
#include "Server.h"
#include "Handler.h"
#include "protocol.h"

int Server::init() {
    pool.init();
    return 0;
}

int Server::shutdown() {
    pool.shutdown();
    return 0;
}

int Server::run(int port)
{
    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "listen fd=" << listenfd << std::endl; 


    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // on_exit(handle_signal, (void*)&listenfd);
    // signal(SIGINT, handle_signal);
    
    struct sockaddr_in servaddr;
    bzero((void*)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, 5);

    int epfd = Epoll_create(1);

    // for socket listenfd
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    Epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    struct  epoll_event evlist[MAX_CLIENT_SIZE];
    
    while (1) {
        int count = Epoll_wait(epfd, evlist, MAX_CLIENT_SIZE, -1);
        for (int i=0; i<count; i++) {
            struct epoll_event *one = evlist + i;
            if (one->events & EPOLLIN) {
                int fd = one->data.fd;
                if (listenfd == fd) {
                    struct sockaddr_in clientaddr;
                    socklen_t len = sizeof(clientaddr);
                    int clientfd = Accept(fd, (struct sockaddr*)&clientaddr, &len);
                    std::cout << "enter clientfd " << clientfd << std::endl; 

                    struct epoll_event ev;
                    ev.events = EPOLLIN;
                    ev.data.fd = clientfd;
                    Epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
                    add_client(clientfd, clientaddr);

                } else {
                    if (handle_request(fd) == 0) {
                        Epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
                        Close(fd);
                        remove_client(fd);
                        std::cout << "client exit fd=" << fd << std::endl; 
                    }
                }
            }
        }
    }

    return 0;
}

int writeResponse(int fd, std::string response)
{
    const char *text = response.c_str();
    int len = response.length();
    Write(fd, text, len);
    return 0;
}

int Server::handle_request(int fd) {
    const size_t BUF_SIZE = 512; 
    char buf[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    int n = Read(fd, buf, BUF_SIZE);
    if (n > 0) {
        // 1.先从内核读到ringbufer中
        RingBuffer* rb = clientMap[fd].ringBuffer;
        rb->push(buf, n);

        // 2.解析协议
        // 解析协议头
        ProtocolParser parser;
        ProtocolHeader* header = parser.parse_header(buf, HEADER_SIZE);

        if (true) {
            std::string str(buf+13, header->data_length);
            std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::toupper(c); });
            writeResponse(fd, "server:" + str + "\n");
            return n;
        }

        // 解析协议体

        Message* msg = new Message();
        msg->fd = fd;
        msg->text = std::string(buf);

        // 3. submit to thread pool
        pool.submit(handle_message, msg);
    }
    return n;
}

void handle_message(Message* msg) {
    writeResponse(msg->fd, "server:" + msg->text + "\n");
}

void printMap(std::map<int, Client> map) {
    std::map<int, Client>::iterator it = map.begin();
    while(it != map.end()) {
        int fd = it->first;
        Client cli = it->second;
        std::cout << fd << ", " << cli.fd << std::endl;
        it++;
    }
}

int Server::add_client(int fd, struct sockaddr_in addr) {
    std::cout << "add client " << fd << std::endl;

    Client* client = new Client();
    client->fd = fd;
    client->clientaddr = addr;
    client->ringBuffer = new RingBuffer(RING_BUFFER_SIZE);
    clientMap[fd] = *client;
    // printMap(clientMap);

    return 0;
}

int Server::remove_client(int fd) {
    std::cout << "remove client " << fd << std::endl;
    clientMap.erase(fd);
    // printMap(clientMap);
    return 0;
}
