#include "room.h"
#include <TimerManager.h>
#include <timer.h>
#include <server_push.h>
#include "player.h"

void to_json(nlohmann::json &j, const Room &r)
{
    j = nlohmann::json{
        {"id", r.getId()},
        {"m_state", r.getState()},
        {"preTime", 300},
        {"moveTime", 30},
        {"readSecondCount", 3},
    };
    std::vector<Player> list;
    list.push_back(*r.getBlackPlayer().get());
    list.push_back(*r.getWhitePlayer().get());
    for (const auto &[key, value] : r.getGuests())
    {
        list.push_back(*value);
    }
    j["players"] = list;

    to_json(j["board"], r.getConstBoard());
}

// void from_json(nlohmann::json& j, Room& r) {
//     r.id = j["id"];
//     r.state = j["state"];
// }

bool Room::is_player(std::shared_ptr<Player> p) const
{
    RoomRole role = getRole(p);
    return role == RoomRole::BLACK_PLAYER || role == RoomRole::WHITE_PLAYER;
}

bool Room::is_guest(std::shared_ptr<Player> p) const
{
    RoomRole role = getRole(p);
    return role == RoomRole::GUEST;
}

bool Room::is_counting_selecting(std::shared_ptr<Player> p) const
{
    if (p->color == "B")
    {
        return mPointCountingState & Room::COUNTING_STAT_BLACK_SELECTING;
    }

    if (p->color == "W")
    {
        return mPointCountingState & Room::COUNTING_STAT_WHITE_SELECTING;
    }

    return false;
}

bool Room::is_both_accept() const
{
    bool b_accept = mPointCountingState & Room::COUNTING_STAT_BLACK_ACCEPT;
    bool w_accept = mPointCountingState & Room::COUNTING_STAT_WHITE_ACCEPT;
    return b_accept && w_accept;
}

Board &Room::getBoard()
{
    return mBoard;
}

const Board &Room::getConstBoard() const
{
    return mBoard;
}

std::map<std::string, std::shared_ptr<Player>> Room::getGuests() const
{
    return mGuest;
}

int Room::getState() const
{
    return mState;
}

std::string Room::getId() const
{
    return mId;
}

std::shared_ptr<Player> Room::getBlackPlayer() const
{
    return mBlackPlayer;
}

std::shared_ptr<Player> Room::getWhitePlayer() const
{
    return mWhitePlayer;
}

void Room::setBlackPlayer(std::shared_ptr<Player> p)
{
    mBlackPlayer = p;
}

void Room::setWhitePlayer(std::shared_ptr<Player> p)
{
    mWhitePlayer = p;
}

bool Room::addGuest(std::shared_ptr<Player> player)
{
    mGuest[player->id] = player;
    return true;
}

bool Room::removeGuest(std::shared_ptr<Player> p)
{
    mGuest.erase(p->id);
    return true;
}

RoomRole Room::getRole(std::shared_ptr<Player> p) const
{
    if (p->id == mBlackPlayer->id) return RoomRole::BLACK_PLAYER;
    if (p->id == mWhitePlayer->id) return RoomRole::WHITE_PLAYER;
    if (mGuest.find(p->id) != mGuest.end()) return RoomRole::GUEST;
    return RoomRole::UNKNOW;
}

void onMoveTimeout() {
    std::cout << "invalid game" << std::endl; 
    // 1. push message: invalid game
    // ServerPusher::getInstance().server_push(fd, PushMessage{"move_timeout", {
    //     {"room_id", room_id},
    //     {"player_id", player_id},
    // }});

    // // 2. set room state
    // std::shared_ptr<Room> room = g_rooms[room_id];
    // room->switchRoomState(Room::ROOM_STATE_GAME_OVER);
}

void Room::createGoClock(InitClockTime time)
{
    mGoClock = std::make_shared<GoClock>(time);
    mGoClock->setCallback([](int val) {
        // 倒计时回调
        // 1. 要给room内所有人push一个时间
        // 2. 通过val检查超时
        // onMoveTimeout
        std::cout << "GoClock Callback called!" << std::endl;
    });
}

Room::Room(std::string id)
    : mId(id),
      core(std::make_shared<RoomCore>())
{
}

Room::~Room() {}

void Room::clone(const Room &other)
{
    this->mId = other.mId;
    this->mBlackPlayer = other.mBlackPlayer;
    this->mWhitePlayer = other.mWhitePlayer;
    this->mGuest = other.mGuest;
    this->mState = other.mState;
    this->mBoard = other.mBoard;
    this->mScore = other.mScore;
}

Room::Room(const Room &one)
{
    clone(one);
}

Room &Room::operator=(const Room &one)
{
    if (this == &one)
        return *this;

    clone(one);
    return *this;
}

void Room::start()
{
    mGoClock->start();
}

void Room::switchRoomState(int newState)
{
    if (newState == Room::ROOM_STATE_INIT)
    {
    }
    else if (newState == Room::ROOM_STATE_WAITTING_BLACK_MOVE)
    {
        mGoClock->whiteTap();
    }
    else if (newState == Room::ROOM_STATE_WAITTING_WHITE_MOVE)
    {
        mGoClock->blackTap();
    }
    else if (newState == Room::ROOM_STATE_BLACK_OFFLINE)
    {
        mGoClock->stop();
    }
    else if (newState == Room::ROOM_STATE_WHITE_OFFLINE)
    {
        mGoClock->stop();
    }
    else if (newState == Room::ROOM_STATE_POINT_COUNTTING)
    {
        mGoClock->stop();
    }
    else if (newState == Room::ROOM_STATE_GAME_OVER)
    {
        mGoClock->stop();
    }

    mState = newState;
}

void Room::switchPointCountingState(u_int8_t new_state)
{
    mPointCountingState = new_state;
}