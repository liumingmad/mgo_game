#ifndef PUSH_MESSAGE_H
#define PUSH_MESSAGE_H

#include <string>
#include <vector>
#include <queue>

#include "json.hpp"
#include "body.h"

class PushMessage
{
public:
    std::string action;
    nlohmann::json data;
    PushMessage(std::string action, nlohmann::json data)
    : action(action), data(data) {
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PushMessage, action, data)

#endif // PUSH_MESSAGE_H