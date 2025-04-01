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
    if (n == 0) {
        return 0;
    }

    // 查看ringbuffer中是否残存数据, 如果存在，就沾包
    RingBuffer* rb = clientMap[fd].ringBuffer;
    rb->push(buf, n);

    ProtocolParser parser;
    while (true) {
        // 1.在buffer中，检查是否存在一条完整的协议
        size_t len = parser.exist_one_protocol(rb);
        if (len == 0) {
            break;
        }

        // 2.解析协议头
        ProtocolHeader* header = parser.parse_header(buf, HEADER_SIZE);

        // 3.把数据放入Message, 交给线程池处理
        Message* msg = new Message();
        msg->fd = fd;
        msg->text = std::string(buf+13, header->data_length);
        pool.submit(handle_message, msg);

        // 4. 删除处理过的数据
        rb->pop(nullptr, header->data_length + HEADER_SIZE);
    }

    // 清理无效数据
    parser.clear_invalid_data(rb);

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
