#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <map>
#include <iostream>
#include "event_handler.h"
#include <wrap.h>
#include <common_utils.h>
#include <global.h>
#include <redis_pool.h>

int EventHandler::init()
{
    mEventfd = eventfd(0, EFD_NONBLOCK);
    return 0;
}

int EventHandler::getEventfd() const
{
    return mEventfd;
}

void EventHandler::post(std::shared_ptr<EVMessage> msg)
{
    mConcurrentQueue.enqueue(msg);
    eventfd_write(mEventfd, 1);
}

void EventHandler::cleanEventfd() {
    eventfd_t val;
    eventfd_read(mEventfd, &val);
}

moodycamel::ConcurrentQueue<std::shared_ptr<EVMessage>>& EventHandler::getConcurrentQueue() 
{
    return mConcurrentQueue;
}
