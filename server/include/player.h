#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <queue>
#include "json.hpp"

class Player
{
public:
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

    
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, id, name, level, color)

#endif // PLAYER_H