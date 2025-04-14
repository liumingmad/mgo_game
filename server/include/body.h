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
    // B or W char code
    std::string color;

    // 上线和离线的时间点列表
    std::queue<long> active_time_list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, id, name, level, color)

struct Room
{
    std::string id;
    std::vector<Player> players;

    // 使用二进制位表示
    // 0: 等待黑棋落子
    // 1: 等待白棋落子
    // 2: 黑方离线暂停
    // 3: 白方离线暂停
    // 4: 双方全都离线暂停
    // 5: 正在数子
    // 6: 对局结束
    int state;

    // Board board;
    
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Room, id, players, state)

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