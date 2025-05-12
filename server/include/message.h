#ifndef HANDLE_H
#define HANDLE_H


#include <vector>
#include <string>
#include <memory>
#include "protocol.h"
#include "body.h"
#include "player.h"

class Room;

class Message {
public:
    // client_id
    int cid;

    // 发本条消息的人
    std::shared_ptr<Player> self;

    std::shared_ptr<ProtocolHeader> header;
    std::shared_ptr<Request> request;
};

class RoomMessage {
public:
    std::shared_ptr<Message> reqMsg;
    std::shared_ptr<Room> room;
    std::string action;
    std::any data;
};

#endif // HANDLE_H