#include "room.h"

void to_json(nlohmann::json& j, const Room& r) {
    j = nlohmann::json{
        {"id", r.id},
        {"m_state", r.state},
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

// void from_json(const nlohmann::json& j, Board& b) {
//     b.setWidth(j.at("width").get<int>());
//     b.setHeight(j.at("height").get<int>());
//     if (j.contains("root")) {
//         auto root = new Node();
//         j.at("root").get_to(*root);
//         b.setRoot(root);
//     }
// }