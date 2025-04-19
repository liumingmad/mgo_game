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
    j = nlohmann::json{
        {"x", s.x},
        {"y", s.y},
        {"color", s.color},
    };
}

void from_json(const nlohmann::json& j, Stone& s) {
    s.x = j["x"];
    s.y = j["y"];
    s.color = j["color"].get<std::string>().at(0);
}