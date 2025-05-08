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

#endif // PUSH_MESSAGE_H