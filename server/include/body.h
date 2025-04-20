#ifndef BODY_H
#define BODY_H

#include <string>
#include <vector>
#include <queue>

#include "json.hpp"
#include "Board.h"
// #include "room.h"

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
    // B or W char code
    // X is guest
    std::string color = "X";

    // 开始申请的时间戳
    long mills;

    // online
    bool online;

    // 上线和离线的时间点列表
    std::queue<long> active_time_list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, id, name, level, color)

// struct Room
// {
//     std::string id;
//     std::vector<Player> players;
//     int state;
//     Board board;
    
// };
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Room, id, players, state, board)

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

struct MatchPlayerRequest
{
    int level;
    int preTime;
    int readSecondCount;
    int moveTime;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MatchPlayerRequest, level, preTime, readSecondCount, moveTime)

#endif // BODY_H

