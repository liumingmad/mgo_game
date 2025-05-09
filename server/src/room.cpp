#include "room.h"
#include <TimerManager.h>
#include <timer.h>
#include "server_push.h"
#include "player.h"
#include "start_game_body.h"
#include "common_utils.h"
#include <global.h>

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

RoomRole Room::getRole(std::shared_ptr<Player> p) const
{
    if (p->id == mBlackPlayer->id) return RoomRole::BLACK_PLAYER;
    if (p->id == mWhitePlayer->id) return RoomRole::WHITE_PLAYER;
    if (mGuest.find(p->id) != mGuest.end()) return RoomRole::GUEST;
    return RoomRole::UNKNOW;
}

std::shared_ptr<GoClock> Room::getGoClock() {
    return mGoClock;
}



void Room::createGoClock(InitClockTime time)
{
    mGoClock = std::make_shared<GoClock>(time);
    mGoClock->setCallback([&]() {
        std::shared_ptr<RoomMessage> rmsg = std::make_shared<RoomMessage>();
        rmsg->action = "clock_tick";
        rmsg->data = mGoClock;
        postRoomMessage(rmsg);
    });
}

Room::Room(std::string id)
    : mId(id),
      core(std::make_shared<RoomCore>())
{
    core->room = shared_from_this();
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

void Room::pushMessageToAll(std::shared_ptr<PushMessage> pmsg) const {
    ServerPusher::getInstance().server_push(getBlackPlayer()->id, pmsg); 
    ServerPusher::getInstance().server_push(getWhitePlayer()->id, pmsg); 

    const std::map<std::string, std::shared_ptr<Player>>& map = getGuests();
    for (const auto& [uid, value] : map) {
        ServerPusher::getInstance().server_push(uid, pmsg);
    }
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


// 给两个人分别推送一条消息, 客户端之间跳转到room page开始下棋
void Room::pushStartGame() {
    StartGameBody body(*this);
    std::shared_ptr<PushMessage> pmsg = std::make_shared<PushMessage>("start_game", body);
    pushMessageToAll(pmsg);
}

void Room::pushMove() {
    // 获取最新的一步棋
    const Stone& stone = mBoard.getCurrentNode()->getStone();

    std::string player_id;
    if (stone.color == 'B') {
        player_id = mBlackPlayer->id;
    } else if (stone.color == 'W') {
        player_id = mWhitePlayer->id;
    }

    nlohmann::json j = {
        {"room_id", getId()},
        {"player_id", player_id},
        {"stone", stone},
        {"board", getBoard()}
    };

    // 推送落子到room内所有人
    std::shared_ptr<PushMessage> pmsg = std::make_shared<PushMessage>("move", j);
    pushMessageToAll(pmsg);
}

// 所有触发room state改变的事件，需要通知room内所有人
// 0. 匹配对手成功
// 1. 黑/白棋落子
// 2. 黑/白方离线
// 3. guest 进入/离开
// 4. 申请点目
// 5. 接受或拒绝点目结果
// 6. 认输


void Room::switchRoomState(int newState)
{
    Log::info("switchRoomState oldState=" + mState);
    Log::info("switchRoomState newState=" + newState);
    if (mState == Room::ROOM_STATE_INIT)
    {
        if (newState == Room::ROOM_STATE_WAITTING_BLACK_MOVE) { // 等待第一步棋
            pushStartGame();
            mGoClock->start();
        }
    }
    else if (mState == Room::ROOM_STATE_WAITTING_BLACK_MOVE)
    {
        // 黑棋下完，等白棋落子
        if (newState == Room::ROOM_STATE_WAITTING_WHITE_MOVE) {
            pushMove();
            mGoClock->resumeWClock();
        }
    }
    else if (mState == Room::ROOM_STATE_WAITTING_WHITE_MOVE)
    {
        // 白棋下完，等黑棋落子
        if (newState == Room::ROOM_STATE_WAITTING_WHITE_MOVE) {
            pushMove();
            mGoClock->resumeBClock();
        }
    }
    else if (mState == Room::ROOM_STATE_BLACK_OFFLINE)
    {
        // 离线倒记时
        mGoClock->stop();
    }
    else if (mState == Room::ROOM_STATE_WHITE_OFFLINE)
    {
        // 离线倒记时
        mGoClock->stop();
    }
    else if (mState == Room::ROOM_STATE_POINT_COUNTTING)
    {
        mGoClock->stop();
    }
    else if (mState == Room::ROOM_STATE_GAME_OVER)
    {
        mGoClock->stop();
    }

    mState = newState;
}

void Room::switchPointCountingState(u_int8_t new_state)
{
    mPointCountingState = new_state;
}

void Room::handleRoomMessage(std::shared_ptr<RoomMessage> msg) {
}

void Room::postRoomMessage(std::shared_ptr<RoomMessage> msg) {
    queue.enqueue(msg);
    g_room_pool.submit([&]() {
        // 加锁的原因是，不想让多个线程同时处理一个room的消息
        std::unique_lock<std::mutex> lock(mutex);
        while (!queue.empty()) {
            std::shared_ptr<RoomMessage> msg;
            if (queue.dequeue(msg)) {
                core->run(msg);
            }
        }
    });
}