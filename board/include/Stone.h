#ifndef STONE_H
#define STONE_H

#include "json.hpp"

class Stone
{
    public:
        int x;
        int y;
        // 'B' = black, 'W' = white
        char color;
        Stone();
        Stone(int x, int y, char player);
        ~Stone();
};

void to_json(nlohmann::json& j, const Stone& s);
void from_json(const nlohmann::json& j, Stone& s);


#endif // STONE_H