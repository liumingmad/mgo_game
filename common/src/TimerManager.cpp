#include "TimerManager.h"
#include <unistd.h>
#include <string.h>
#include <iostream>

TimerManager& TimerManager::instance() {
    static TimerManager instance;
    return instance;
}

TimerManager::TimerManager() {
    timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
}

TimerManager::~TimerManager() {
    close(timerFd);
}

void TimerManager::init(int epollFd_) {
    epollFd = epollFd_;
    epoll_event ev = {0};
    ev.events = EPOLLIN;
    ev.data.fd = timerFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, timerFd, &ev);
}

void TimerManager::addTask(int id, uint64_t delayMs, function<void()> cb) {
    lock_guard<mutex> lock(mtx);
    uint64_t expireTime = nowMs() + delayMs;
    tasks.push(TimerTask{id, expireTime, cb});
    taskMap[id] = expireTime;
    resetTimer();
}

void TimerManager::removeTask(int id) {
    lock_guard<mutex> lock(mtx);
    taskMap.erase(id);
    resetTimer();
}

void TimerManager::handleTimerEvent() {
    uint64_t exp;
    read(timerFd, &exp, sizeof(exp));

    lock_guard<mutex> lock(mtx);
    uint64_t currentTime = nowMs();
    while (!tasks.empty()) {
        auto task = tasks.top();
        if (task.expire > currentTime) break;
        tasks.pop();
        if (taskMap.count(task.id) && taskMap[task.id] == task.expire) {
            task.callback();
            taskMap.erase(task.id);
        }
    }
    resetTimer();
}

int TimerManager::getTimerFd() {
    return timerFd;
}

void TimerManager::resetTimer() {
    itimerspec newValue{};
    if (tasks.empty()) {
        newValue.it_value.tv_sec = 0;
        newValue.it_value.tv_nsec = 0;
    } else {
        uint64_t diff = tasks.top().expire > nowMs() ? tasks.top().expire - nowMs() : 0;
        newValue.it_value.tv_sec = diff / 1000;
        newValue.it_value.tv_nsec = (diff % 1000) * 1000000;
    }
    timerfd_settime(timerFd, 0, &newValue, nullptr);
}

uint64_t TimerManager::nowMs() {
    return chrono::duration_cast<Milliseconds>(Clock::now().time_since_epoch()).count();
}