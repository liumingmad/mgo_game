#ifndef PUSH_MESSAGE_H
#define PUSH_MESSAGE_H

#include <string>
#include <vector>
#include <queue>

#include "json.hpp"
#include "body.h"
#include "room.h"

struct PushMessage
{
    std::string action;
    nlohmann::json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PushMessage, action, data)

class StartGameBody
{
public:
    Room room;
    StartGameBody(Room r):room(r) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartGameBody, room)

#endif // PUSH_MESSAGE_H