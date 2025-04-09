#ifndef BODY_H
#define BODY_H

#include <string>
#include <vector>
#include <queue>

#include "json.hpp"
#include "Board.h"

struct Request
{
    std::string token;
    std::string action;
    nlohmann::json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Request, token, action, data)

struct Response
{
    int code;
    std::string message;
    nlohmann::json data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Response, code, message, data)

struct Player
{
    std::string id;
    std::string name;
    int level;
    // 开始申请的时间戳
    long mills;
    int online;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, id, name, level, online)


struct Room
{
    std::string id;
    std::vector<Player> players;
    // Board board;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Room, id, players)

struct SignInResponse
{
    std::string token;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SignInResponse, token)


struct MatchPlayerResponse
{
    int waiting_seconds;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MatchPlayerResponse, waiting_seconds)
#endif // BODY_H