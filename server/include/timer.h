#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

// 回调接口
class TimerCallback
{
public:
    virtual void onTrigger() = 0;
    virtual ~TimerCallback() = default;
};

class GameTimerCallback : public TimerCallback
{
public:
    void onTrigger() override;
};

// Timer 类
class Timer
{
private:
    long m_drution;
    std::thread m_thread;
    std::atomic<bool> m_running{true};
    std::vector<std::shared_ptr<TimerCallback>> m_listener_list;

public:
    Timer(const long durtion = 1000):m_drution(durtion)
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

    void addListener(std::shared_ptr<TimerCallback> listener)
    {
        m_listener_list.push_back(listener);
    }

    void start()
    {
        m_thread = std::thread([this]()
                               {
            while (m_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_drution));
                for (const auto& callback : m_listener_list) {
                    if (callback) {
                        callback->onTrigger();
                    }
                }
                // std::cout << "定时器触发任务: " << std::time(nullptr) << std::endl;
            } });
    }

    void cancel()
    {
        m_running = false;
    }
};

#endif // TIMER_H