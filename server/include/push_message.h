#ifndef PUSH_MESSAGE_H
#define PUSH_MESSAGE_H

#include <string>
#include <vector>
#include <queue>

#include "json.hpp"
#include "body.h"

struct PushMessage
{
    std::string action;
    nlohmann::json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PushMessage, action, data)

struct StartGameBody
{
    Room room;
    int preTime; // 5mins
    int moveTime; // 60 seconds
    int readSecondCount; // 读秒次数
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartGameBody, room, preTime, moveTime, readSecondCount)

#endif // PUSH_MESSAGE_H