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
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, id, name, level)


struct Room
{
    std::string id;
    std::vector<Player> players;
    // Board board;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Room, id, players)


#endif // BODY_H