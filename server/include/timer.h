#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

#include "TimerManager.h"

class Timer
{
private:
    std::thread m_thread;
    std::atomic<bool> m_running{true};

public:
    static const int TIME_TASK_ID_HEARTBEAT = 1; 
    static const int TIME_TASK_ID_WAITTING_MOVE = 2; 
    static const int TIME_TASK_ID_COUNTDOWN = 3;

    Timer()
    {
    }

    ~Timer()
    {
        cancel(); // 停止定时器
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void start()
    {
        m_thread = std::thread([this]()
        {
            int epfd = epoll_create1(0);
            TimerManager::instance().init(epfd);
            epoll_event events[10];
            while (m_running) {
                int n = epoll_wait(epfd, events, 10, -1);
                for (int i = 0; i < n; ++i) {
                    if (events[i].data.fd == TimerManager::instance().getTimerFd()) {
                        TimerManager::instance().handleTimerEvent();
                    }
                }
            } 
        });
    }

    void cancel()
    {
        m_running = false;
    }
};

#endif // TIMER_H