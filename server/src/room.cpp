#include "room.h"
#include <TimerManager.h>
#include <timer.h>
#include <server_push.h>

void to_json(nlohmann::json& j, const Room& r) {
    j = nlohmann::json{
        {"id", r.id},
        {"m_state", r.state},
        {"preTime", 300},
        {"moveTime", 30},
        {"readSecondCount", 3},
    };
    std::vector<Player> list;
    for (const auto& [key, value] : r.players) {
        list.push_back(*value);
    }
    j["players"] = list; 

    to_json(j["board"], r.board);
}

// void from_json(nlohmann::json& j, Room& r) {
//     r.id = j["id"];
//     r.state = j["state"];
// }

bool Room::is_player(std::shared_ptr<Player> p) const {
    return p->color == "B" || p->color == "W";
}

bool Room::is_guest(std::shared_ptr<Player> p) const {
    return !is_player(p);
}

bool Room::is_counting_selecting(std::shared_ptr<Player> p) const {
    if (p->color == "B") {
        return m_point_counting_state & Room::COUNTING_STAT_BLACK_SELECTING;
    }

    if (p->color == "W") {
        return m_point_counting_state & Room::COUNTING_STAT_WHITE_SELECTING;
    }

    return false;
}

bool Room::is_both_accept() const {
    bool b_accept = m_point_counting_state & Room::COUNTING_STAT_BLACK_ACCEPT;
    bool w_accept = m_point_counting_state & Room::COUNTING_STAT_WHITE_ACCEPT;
    return b_accept && w_accept;
}

void Room::countdown() {
    // push给room内所有人
    // ServerPusher::getInstance().server_push(fd, PushMessage{"countdown", {
    //     {"room_id", room_id},
    // }});

    TimerManager::instance().addTask(Timer::TIME_TASK_ID_COUNTDOWN, 3000, [&](){
        countdown();
    });
}

void Room::switchRoomState(int newState) {
    this->state = newState;
}