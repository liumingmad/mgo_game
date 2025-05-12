#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <queue>
#include "json.hpp"

constexpr int PLAYER_OFFLINE = 0;
constexpr int PLAYER_ONLINE = 1;
constexpr int PLAYER_WAITTING_REBACK = 2;

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
    int state = PLAYER_ONLINE;

    Player() = default;

    ~Player() = default;

    void clone(const Player &other)
    {
        this->id = other.id;
        this->name = other.name;
        this->level = other.level;
        this->color = other.color;
        this->mills = other.mills;
        this->state = other.state;
    }

    Player(const Player &one)
    {
        clone(one);
    }

    Player &operator=(const Player &one)
    {
        if (this == &one)
            return *this;

        clone(one);
        return *this;
    } 

    Player(Player &&) = default;
    Player &operator=(Player &&) = default;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, id, name, level, color)

#endif // PLAYER_H