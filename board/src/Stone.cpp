#include "Stone.h"

Stone::Stone() {

}

Stone::Stone(int x, int y, char color) {
    this->x = x;
    this->y = y;
    this->color = color;
}

Stone::~Stone() {
}

void to_json(nlohmann::json& j, const Stone& s) {
    std::string color_str;
    if (s.color == 'T') {
        color_str = "T";
    } else if (s.color == 'B') {
        color_str = "B";
    } else if (s.color == 'W') {
        color_str = "W";
    } else {
        color_str = "X";
    }

    j = nlohmann::json{
        {"x", s.x},
        {"y", s.y},
        {"color", color_str},
    };
}

void from_json(const nlohmann::json& j, Stone& s) {
    s.x = j["x"];
    s.y = j["y"];
    s.color = j["color"].get<std::string>().at(0);
}