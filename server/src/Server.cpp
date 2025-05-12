#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include "wrap.h"
#include "Server.h"
#include "message.h"
#include "protocol.h"
#include "Log.h"
#include "core.h"
#include "db_connection_pool.h"
#include "redis_pool.h"
#include "timer.h"
#include "global.h"
#include <TimerManager.h>
#include <signal.h>
#include <common_utils.h>
#include <event_handler.h>

// 这三个涉及多线程修改
ThreadSafeUnorderedMap<std::string, std::shared_ptr<Player>> g_players;
ThreadSafeUnorderedMap<std::string, std::shared_ptr<Room>> g_rooms;
ThreadSafeUnorderedMap<std::string, std::shared_ptr<Client>> g_uidClientMap;

// 这两个只在epoll线程修改
ThreadSafeUnorderedMap<int, std::shared_ptr<Client>> g_clientMap;
ThreadSafeUnorderedMap<int, std::shared_ptr<Client>> g_clientIdMap;

ThreadPool g_room_pool;
EventBus g_EventBus;
long global_client_id = 0;


// 怎样发现客户端掉线了？
// 两种情况：
// 1. 正常断开，客户端发fin
//      1.当服务端发送fin之前，调用read，返回0, write正常
//      2.当服务端发送fin后，调用read，会返回-1 && errno == EBADF

// 2. 异常断开，客户端TCP状态机处于close状态，
//      当服务器调用read时，如果内核读缓冲没东西，则阻塞
//      当服务器调用write时，客户端会回复RST, 服务器收到后，会给进程发送SIGPIPE
//          1.没忽略SIGPIPE, 默认会杀进程, 
//          2.如果忽略了SIGPIPE, write会立刻返回-1，errno == EPIPE

// 正常断开比较容易处理，
// 当异常断开时，为了能早点发现客户端close了，就要检查客户端心跳包，如果超时没收到，直接调用close, 把fd踢掉

// 当给客户端写消息时，如果内核缓冲区满了，write阻塞了怎么办
// 用epoll监听，EPOLLOUT, 分段去写, 在水平触发下，写完了要取消监听

// 问题1：处理心跳超时的线程切换
// 问题2: 处理写阻塞, 加了写缓冲，如果内核缓冲满了，等到可写，继续发送
// 解决办法，用epoll监听一个消息队列，把这两个问题，分别包装成消息，发送到epoll线程处理

int Server::init() {
    // 忽略 SIGPIPE，防止进程因 write 出错而崩溃
    // 忽略这个信号后，write时，会返回-1，errno == EPIPE
    signal(SIGPIPE, SIG_IGN);

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

    // 处理room外request的线程池
    m_pool.init();

    // 处理room内request的线程池
    g_room_pool.init();

    // 事件总线
    AsyncEventBus& bus = AsyncEventBus::getInstance();
    bus.start();

    EventHandler::getInstance().init();

    // 定时器
    m_timer.start();

    return 0;
}

int Server::shutdown() {
    m_pool.shutdown();
    g_room_pool.shutdown();
    AsyncEventBus::getInstance().stop();
    return 0;
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int Server::run(int port)
{
    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listenfd);
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
    epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    Epoll_ctl(m_epfd, EPOLL_CTL_ADD, listenfd, &ev);

    // for eventfd
    int eventfd = EventHandler::getInstance().getEventfd();
    ev.events = EPOLLIN;
    ev.data.fd = eventfd;
    Epoll_ctl(m_epfd, EPOLL_CTL_ADD, eventfd, &ev);

    struct  epoll_event evlist[MAX_CLIENT_SIZE];
    
    while (1) {
        int count = Epoll_wait(m_epfd, evlist, MAX_CLIENT_SIZE, -1);
        for (int i=0; i<count; i++) {
            struct epoll_event *one = evlist + i;
            int fd = one->data.fd;
            if (listenfd == fd) {
                handleListenfd(fd);
            } else if (fd == eventfd) {
                handleEventfd();
            } else {
                if (one->events & EPOLLIN) {
                    handle_read(fd);
                } else if (one->events & EPOLLOUT) {
                    handle_write(fd);  // 对客户端继续写未完成的数据
                }
            }
        }
    }

    return 0;
}

std::string genTimerId(int fd) {
    return TIME_TASK_ID_HEARTBEAT + to_string(fd);
}

void Server::handleListenfd(int fd) {
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = Accept(fd, (struct sockaddr*)&clientaddr, &len);
    set_nonblocking(clientfd);
    std::cout << "enter clientfd " << clientfd << std::endl; 

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientfd;
    Epoll_ctl(m_epfd, EPOLL_CTL_ADD, clientfd, &ev);
    add_client(clientfd, clientaddr);
}

std::shared_ptr<Request> parseRequest(const std::string& jsonStr) {
    try {
        nlohmann::json j = nlohmann::json::parse(jsonStr);
        auto request = std::make_shared<Request>(j.get<Request>());
        std::cout << "解析成功: " << j.dump(2) << std::endl;
        return request;
    }
    catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON 解析错误: " << e.what() << ", 字节 " << e.byte << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
    return nullptr;
}

int Server::handle_read(int fd) {
    Log::info("\n\n-----------START-----------------");
    auto client = g_clientMap.get(fd).value();

    char buf[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    int n = Read(fd, buf, BUF_SIZE);
    if (n <= 0) {
        Epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
        Close(fd);
        remove_client(fd);
        std::cout << "client exit fd=" << fd << std::endl; 
    }

    // 读到任何东西，都认为客户端活着
    client->activeTimestamp = get_now_milliseconds();
    // 如果超过30s，客户端没有任何活动，就超时
    TimerManager::instance().addTask(genTimerId(fd), 30000, [fd](){
        auto msg = std::make_shared<EVMessage>(fd, EVMESSAGE_TYPE_HEARTBEAT_TIMEOUT, nullptr);
        EventHandler::getInstance().post(msg);
    });

    // 查看ringbuffer中是否残存数据, 如果存在，就沾包
    std::shared_ptr<RingBuffer> rb = client->ringBuffer;
    rb->push(buf, n);

    // 1. 可能是http协议，需要处理登录/注册/profile
    // 2. 可能是websocket协议，data部分是mogo协议，从web端过来的数据
    // 3. 可能是mogo协议
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
        msg->cid = client->id;
        if (!(client->user_id.empty())) {
            msg->self = g_players.get(client->user_id).value();
        }
        msg->header = header;
        msg->request = parseRequest(std::string(tmp+HEADER_SIZE, header->data_length));
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
            if (msg->header && msg->header->command == ProtocolHeader::HEADER_COMMAND_HEART) {
                response_heartbeat(msg->cid);
                continue;
            }
            client->core->run(msg);
        }
    }
}

void Server::handleHeartBeatTimerout(int fd) {
    Epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
    Close(fd);
    remove_client(fd);
}

void Server::handle_write(int fd)
{
    auto opt = g_clientMap.get(fd);
    assert(opt.has_value());
    std::shared_ptr<Client> client = opt.value(); 

    auto &buf = client->pendingWriteBuffer;
    ssize_t n = write(fd, buf.data(), buf.size());
    if (n > 0)
    {
        buf.erase(0, n); // 移除已发送部分
        if (buf.empty())
        {
            // 写完了，取消 EPOLLOUT 监听，避免 busy loop
            epoll_event ev{};
            ev.events = EPOLLIN;
            ev.data.fd = fd;
            epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
        }
    }
    else if (n == -1 && (errno != EAGAIN && errno != EWOULDBLOCK))
    {
        // 真正的错误（如连接断开）
        perror("write failed");
        Epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        remove_client(fd);
    }
}

void Server::schedule_write(int fd, const std::string &data)
{
    auto opt = g_clientMap.get(fd);
    assert(opt.has_value());
    std::shared_ptr<Client> client = opt.value();

    client->pendingWriteBuffer += data; // 缓存数据

    // 尝试立即写
    handle_write(fd);

    // 如果还有剩余，监听 EPOLLOUT
    if (!client->pendingWriteBuffer.empty())
    {
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.fd = fd;
        epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
    }
}

int Server::add_client(int fd, struct sockaddr_in addr) {
    std::shared_ptr<Client> client = std::make_shared<Client>();
    std::cout << "add client id " << client->id << std::endl;
    client->fd = fd;
    client->clientaddr = addr;
    client->activeTimestamp = get_now_milliseconds();
    g_clientMap.set(fd, client);
    g_clientIdMap.set(client->id, client);

    auto req = std::make_shared<Request>();
    req->action = "online";
    std::shared_ptr<Message> msg = std::make_shared<Message>();
    msg->cid = client->id;
    if (!(client->user_id.empty())) {
        msg->self = g_players.get(client->user_id).value();
    }
    msg->request = req;
    client->queue.enqueue(msg);
    m_pool.submit([this, client]() {
        this->handle_message(client);
    });
}

// 上线/离线也做成消息，发送到client队列处理
int Server::remove_client(int fd) {
    auto opt = g_clientMap.get(fd);
    assert(opt.has_value());
    std::shared_ptr<Client> client = opt.value();
    std::cout << "remove client fd " << fd << ", id "<< client->id << std::endl;
    
    auto req = std::make_shared<Request>();
    req->action = "offline";
    std::shared_ptr<Message> msg = std::make_shared<Message>();
    msg->cid = client->id;
    if (!(client->user_id.empty())) {
        msg->self = g_players.get(client->user_id).value();
    }
    msg->request = req;
    client->queue.enqueue(msg);
    m_pool.submit([this, client]() {
        this->handle_message(client);
    });

    return 0;
}

void Server::handleEventfd()
{
    EventHandler& handler = EventHandler::getInstance();
    auto& queue = handler.getConcurrentQueue();
    // dequeue
    std::shared_ptr<EVMessage> msg;
    while (queue.try_dequeue(msg))
    {
        auto client = g_clientIdMap.get(msg->cid);
        if (!client.has_value()) {
            continue;
        }
        int fd = client.value()->fd;

        switch (msg->type)
        {
        case EVMESSAGE_TYPE_HEARTBEAT_TIMEOUT:
            handleHeartBeatTimerout(fd);
            break;

        case EVMESSAGE_TYPE_SERVER_PUSH: {
            auto data = std::any_cast<std::shared_ptr<std::string>>(msg->data);
            std::cout << "EVMESSAGE_TYPE_SERVER_PUSH fd=" << fd << ", cid=" << client.value()->id << std::endl;
            std::cout << (data->c_str()+HEADER_SIZE) << std::endl;
            schedule_write(fd, *data);

            Log::info("\n\n-----------END-----------------");
            break;
        }

        default:
            break;
        }
    }

    // clean flag
    handler.cleanEventfd();
}