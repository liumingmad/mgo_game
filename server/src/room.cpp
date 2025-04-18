#include "room.h"

void to_json(nlohmann::json& j, const Room& r) {
    j = nlohmann::json{
        {"id", r.id},
        {"m_state", r.state},
        {"preTime", r.preTime},
        {"moveTime", r.moveTime},
        {"readSecondCount", r.readSecondCount},
    };
    std::vector<Player> list;
    for (const auto& [key, value] : r.players) {
        list.push_back(value);
    }
    j["players"] = list; 

    to_json(j["board"], r.board);
}

// void from_json(nlohmann::json& j, Room& r) {
//     r.id = j["id"];
//     r.state = j["state"];
// }