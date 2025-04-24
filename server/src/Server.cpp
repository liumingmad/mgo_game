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
#include "Log.h"
#include "core.h"
#include "db_connection_pool.h"
#include "redis_pool.h"
#include "timer.h"
#include "global.h"
#include <TimerManager.h>
#include <heartbeat.h>

std::map<std::string, std::shared_ptr<Player>> g_players;
std::map<std::string, std::shared_ptr<Room>> g_rooms;
std::map<int, std::shared_ptr<Client>> clientMap;
std::map<std::string, std::shared_ptr<Client>> uidClientMap;

int Server::init() {
    // 初始化数据库连接池
    DBConnectionPool::getInstance()->initPool(
        "tcp://172.17.0.1:3306",
        "root",
        "123456",
        "mgo",
        5); // 初始连接数设为15
    
    RedisPool& redisPool = RedisPool::getInstance();
    redisPool.init();
    redisPool.getRedis().flushall();

    // 线程池
    m_pool.init();

    // 事件总线
    AsyncEventBus& bus = AsyncEventBus::getInstance();
    bus.start();

    m_event_handler.init();

    // 定时器
    // m_timer.start();

    return 0;
}

int Server::shutdown() {
    m_pool.shutdown();
    AsyncEventBus::getInstance().stop();
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

    m_epfd = Epoll_create(1);

    // for socket listenfd
    m_epoll_event.events = EPOLLIN;
    m_epoll_event.data.fd = listenfd;
    Epoll_ctl(m_epfd, EPOLL_CTL_ADD, listenfd, &m_epoll_event);

    struct  epoll_event evlist[MAX_CLIENT_SIZE];
    
    while (1) {
        int count = Epoll_wait(m_epfd, evlist, MAX_CLIENT_SIZE, -1);
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
                    Epoll_ctl(m_epfd, EPOLL_CTL_ADD, clientfd, &ev);
                    add_client(clientfd, clientaddr);

                } else {
                    if (handle_request(clientMap[fd]) == 0) {
                        Epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &m_epoll_event);
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

int Server::handle_request(std::shared_ptr<Client> client) {
    char buf[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    int n = Read(client->fd, buf, BUF_SIZE);
    if (n == 0) {
        return 0;
    }

    // 查看ringbuffer中是否残存数据, 如果存在，就沾包
    std::shared_ptr<RingBuffer> rb = client->ringBuffer;
    rb->push(buf, n);

    ProtocolParser parser;
    while (true) {
        // 1.在buffer中，检查是否存在一条完整的协议, 返回协议的全长
        size_t len = parser.exist_one_protocol(rb);
        if (len == 0) {
            break;
        }

        // 2.解析协议头
        std::shared_ptr<ProtocolHeader> header = parser.parse_header(rb, HEADER_SIZE);

        // 3.把数据放入Message, 交给线程池处理
        char tmp[len];
        rb->peek(tmp, len);
        std::shared_ptr<Message> msg = std::make_shared<Message>();
        msg->fd = client->fd;
        msg->header = header;
        msg->text = std::string(tmp+HEADER_SIZE, header->data_length);
        client->queue.enqueue(msg);

        // 4. 删除处理过的数据
        rb->pop(nullptr, len);
    }

    m_pool.submit([this, client]() {
        this->handle_message(client);
    });

    // 清理无效数据
    parser.clear_invalid_data(rb);

    return n;
}

void Server::handle_message(std::shared_ptr<Client> client) {
    // 加锁的原因是，不想让多个线程同时处理一个连接的消息
    std::unique_lock<std::mutex> lock(client->mutex);
    SafeQueue<std::shared_ptr<Message>>& queue = client->queue;
    while (!queue.empty()) {
        std::shared_ptr<Message> msg;
        if (queue.dequeue(msg)) {
            if (msg->header->command == ProtocolHeader::HEADER_COMMAND_HEART) {
                int fd = msg->fd;
                TimerManager::instance().addTask(Timer::TIME_TASK_ID_HEARTBEAT, 30000, [this, fd](){
                    this->heart_timeout(fd);
                });
                std::cout << msg->fd << ":" << msg->text << std::endl;
                response_heartbeat(msg->fd);
            } else {
                client->core->run(*msg);
            }
        }
    }
}

void printMap(std::map<int, std::shared_ptr<Client>> map) {
    std::map<int, std::shared_ptr<Client>>::iterator it = map.begin();
    while(it != map.end()) {
        int fd = it->first;
        std::shared_ptr<Client> cli = it->second;
        std::cout << fd << ", " << cli->fd << std::endl;
        it++;
    }
}

void Server::heart_timeout(int fd) {
    Epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &m_epoll_event);
    Close(fd);
    this->remove_client(fd);
    std::cout << "heart timeout" << std::endl;
}

int Server::add_client(int fd, struct sockaddr_in addr) {
    std::cout << "add client " << fd << std::endl;

    std::shared_ptr<Client> client = std::make_shared<Client>();
    client->fd = fd;
    client->clientaddr = addr;
    clientMap.insert({fd, client});
    // printMap(clientMap);

    // TimerManager::instance().addTask(Timer::TIME_TASK_ID_HEARTBEAT, 30000, [this, fd](){
    //     heart_timeout(fd);
    // });
    return 0;
}

int Server::remove_client(int fd) {
    std::cout << "remove client " << fd << std::endl;

    std::shared_ptr<Client> client = clientMap[fd];
    std::string user_id = client->user_id;

    if (!user_id.empty()) {
        // 删除user_id和client的对应关系
        uidClientMap.erase(client->user_id);

        // 清除redis缓存
        const std::string key_user_id = KEY_USER_PREFIX + user_id;
        Redis &redis = RedisPool::getInstance().getRedis();
        redis.del(key_user_id);

        // auto it = g_players.find(user_id);
        // if (it != g_players.end()) {
        //     it->second.online = false;
        // }
    }

    clientMap.erase(fd);
    // printMap(clientMap);

    AsyncEventBus::getInstance().asyncPublish(EventHandler::EVENT_OFFLINE, user_id);

    return 0;
}

