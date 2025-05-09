#ifndef HANDLE_H
#define HANDLE_H


#include <vector>
#include <string>
#include <memory>
#include "protocol.h"
#include "body.h"

class Message {
public:
    int fd;
    std::shared_ptr<ProtocolHeader> header;
    std::shared_ptr<Request> request;
};

class RoomMessage {
public:
    std::shared_ptr<Message> reqMsg;
    std::string action;
    std::any data;
};

#endif // HANDLE_H