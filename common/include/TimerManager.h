#pragma once

#include <functional>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <sys/epoll.h>
#include <sys/timerfd.h>

using namespace std;
using Clock = chrono::steady_clock;
using Milliseconds = chrono::milliseconds;

struct TimerTask {
    int id;
    uint64_t expire;
    function<void()> callback;
    bool operator>(const TimerTask& other) const {
        return expire > other.expire;
    }
};

class TimerManager {
public:
    static TimerManager& instance();
    void init(int epollFd);
    void addTask(int id, uint64_t delayMs, function<void()> cb);
    void removeTask(int id);
    void handleTimerEvent();
    int getTimerFd();

private:
    TimerManager();
    ~TimerManager();
    void resetTimer();
    uint64_t nowMs();

    int timerFd;
    int epollFd;
    priority_queue<TimerTask, vector<TimerTask>, greater<>> tasks;
    unordered_map<int, uint64_t> taskMap;
    mutex mtx;
};