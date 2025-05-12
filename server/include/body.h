#ifndef BODY_H
#define BODY_H

#include <string>
#include <vector>
#include <queue>

#include "json.hpp"
#include "Board.h"

struct Request
{
    std::string action;
    nlohmann::json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Request, action, data)

struct Response
{
    int code;
    std::string message;
    nlohmann::json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Response, code, message, data)


struct SignInResponse
{
    std::string token;
    std::string tcpToken;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SignInResponse, token)


struct MatchPlayerResponse
{
    int waiting_seconds;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MatchPlayerResponse, waiting_seconds)

struct MatchPlayerRequest
{
    int level;
    int preTime;
    int readSecondCount;
    int moveTime;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MatchPlayerRequest, level, preTime, readSecondCount, moveTime)

#endif // BODY_H

