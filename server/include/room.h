#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <map>
#include "body.h"
#include "json.hpp"
#include "Board.h"
#include "Node.h"
#include "ThreadPool.h"
#include "room_core.h"
#include "player.h"
#include "TimerManager.h"
#include "timer.h"
#include "push_message.h"

struct InitClockTime
{
    int preTime;
    int readSecondCount;
    int moveTime;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitClockTime, preTime, readSecondCount, moveTime)

class RClock
{
public:
    InitClockTime mInitTime;

    // 保留时间
    int mPreTime;

    // 读秒次数
    int mReadSecondCount;

    // 每步棋的的剩余时间
    int mMoveTime;

    RClock(InitClockTime time)
        : mInitTime(time),
          mPreTime(time.preTime),
          mReadSecondCount(time.readSecondCount),
          mMoveTime(time.moveTime)
    {
    }

    ~RClock()
    {
    }

    RClock(RClock &one) = default;
    RClock &operator=(const RClock &one) = default;
    RClock(RClock &&) = default;
    RClock &operator=(RClock &&) = default;

    int getPreTime() const
    {
        return mPreTime;
    }

    int getReadSecondCount() const
    {
        return mReadSecondCount;
    }

    int getMoveTime() const
    {
        return mMoveTime;
    }

    // 减1，超时return true
    bool subOne()
    {
        if (mPreTime > 0)
        {
            mPreTime--;
            return false;
        }

        if (mMoveTime > 0)
        {
            mMoveTime--;
            return false;
        }

        if (mReadSecondCount > 0) 
        {
            mReadSecondCount--;
            mMoveTime = mInitTime.moveTime;
            return false;
        }

        return true;
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RClock, mInitTime, mPreTime, mReadSecondCount, mMoveTime)

class GoClock
{
private:
    std::atomic<bool> mRunning = false;
    std::atomic<bool> mBClockWorking = true;
    std::string mRoomId;
    std::shared_ptr<RClock> mBClock;
    std::shared_ptr<RClock> mWClock;
    std::function<void()> mCallback;

public:
    std::shared_ptr<RClock> getBClock() { return mBClock; }
    std::shared_ptr<RClock> getWClock() { return mWClock; }

    GoClock(InitClockTime time) : mBClock(std::make_shared<RClock>(time)),
                                  mWClock(std::make_shared<RClock>(time))
    {
    }
    ~GoClock() {}

    GoClock(GoClock &one) = default;
    GoClock &operator=(const GoClock &one) = default;
    GoClock(GoClock &&) = default;
    GoClock &operator=(GoClock &&) = default;

    void setCallback(std::function<void()> cb)
    {
        mCallback = cb;
    }

    void countdown()
    {
        // std::shared_ptr<GoClock> goClock = std::make_shared<GoClock>(mBClock, mWClock);
        TimerManager::instance().addTask(mRoomId, 3000, [&]()
                                         {
            if (!mRunning.load(std::memory_order_acquire)) return;

            if (mBClockWorking.load(std::memory_order_acquire)) {
                mBClock->subOne();
            } else {
                mWClock->subOne();
            }

            if (mCallback) {
                mCallback();
            }

            countdown();
            
        });
    }

    void start()
    {
        mRunning.store(true, std::memory_order_release);
        countdown();
    }

    void stop()
    {
        TimerManager::instance().removeTask(mRoomId);
        mRunning.store(false, std::memory_order_release);
    }

    void resumeBClock()
    {
        mBClockWorking.store(true, std::memory_order_release);
    }

    void resumeWClock()
    {
        mBClockWorking.store(false, std::memory_order_release);
    }
};

enum RoomRole
{
    BLACK_PLAYER,
    WHITE_PLAYER,
    GUEST,
    UNKNOW
};

class Room : public std::enable_shared_from_this<Room>
{
private:
    int mState = ROOM_STATE_INIT;
    std::string mId;
    std::shared_ptr<Player> mBlackPlayer;
    std::shared_ptr<Player> mWhitePlayer;
    std::map<std::string, std::shared_ptr<Player>> mGuest;
    Board mBoard;
    Score mScore;
    InitClockTime mInitClockTime;
    std::shared_ptr<GoClock> mGoClock;
    uint8_t mPointCountingState = COUNTING_STAT_INIT;

public:


    std::string getId() const;
    std::shared_ptr<GoClock> getGoClock();
    int getState() const;
    std::map<std::string, std::shared_ptr<Player>> getGuests() const;
    Board &getBoard();
    const Board &getConstBoard() const;

    void setBlackPlayer(std::shared_ptr<Player> p);
    void setWhitePlayer(std::shared_ptr<Player> p);
    std::shared_ptr<Player> getBlackPlayer() const;
    std::shared_ptr<Player> getWhitePlayer() const;
    void createGoClock(InitClockTime time);
    bool addGuest(std::shared_ptr<Player>);
    bool removeGuest(std::shared_ptr<Player>);
    RoomRole getRole(std::shared_ptr<Player>) const;

    void start();

    void pushMessageToAll(std::shared_ptr<PushMessage> pmsg) const;
    void pushStartGame();
    void pushMove(const Stone& stone);

    // 当执行queue中Message的过程中，不能执行队列中下一个
    std::mutex mutex;
    SafeQueue<std::shared_ptr<RoomMessage>> queue;
    std::shared_ptr<RoomCore> core;

    void postRoomMessage(std::shared_ptr<RoomMessage> msg);
    void handleRoomMessage(std::shared_ptr<RoomMessage> msg);

    // 数目的状态
    // 每个人有三种状态：argee/reject/selecting
    // 用6个二进制位表示
    static const u_int8_t COUNTING_STAT_INIT = 0;
    static const u_int8_t COUNTING_STAT_BLACK_ACCEPT = 1;
    static const u_int8_t COUNTING_STAT_BLACK_REJECT = 1 << 1;
    static const u_int8_t COUNTING_STAT_BLACK_SELECTING = 1 << 2;
    static const u_int8_t COUNTING_STAT_WHITE_ACCEPT = 1 << 3;
    static const u_int8_t COUNTING_STAT_WHITE_REJECT = 1 << 4;
    static const u_int8_t COUNTING_STAT_WHITE_SELECTING = 1 << 5;

    void switchPointCountingState(u_int8_t new_state);

    // 使用二进制位表示
    // 0: 等待黑棋落子
    // 1: 等待白棋落子
    // 2: 黑方离线暂停
    // 3: 白方离线暂停
    // 4: 双方全都离线暂停
    // 5: 正在数子
    // 6: 对局结束
    static const int ROOM_STATE_INIT = 0;
    static const int ROOM_STATE_WAITTING_BLACK_MOVE = 1;
    static const int ROOM_STATE_WAITTING_WHITE_MOVE = 1 << 1;
    static const int ROOM_STATE_BLACK_OFFLINE = 1 << 2;
    static const int ROOM_STATE_WHITE_OFFLINE = 1 << 3;
    static const int ROOM_STATE_POINT_COUNTTING = 1 << 4;
    static const int ROOM_STATE_GAME_OVER = 1 << 5;

    void switchRoomState(int newState);

    // 倒计时，每3秒调用一次
    void countdown();

    void init() {
        core->room = shared_from_this();
    }

    static std::shared_ptr<Room> create(const std::string& id) {
        std::shared_ptr<Room> r = std::make_shared<Room>(id);
        r->init();
        return r;
    }

    Room(std::string id)
    : mId(id),
      core(std::make_shared<RoomCore>())
    {}

    ~Room() {}

    void clone(const Room &other)
    {
        this->mId = other.mId;
        this->mBlackPlayer = other.mBlackPlayer;
        this->mWhitePlayer = other.mWhitePlayer;
        this->mGuest = other.mGuest;
        this->mState = other.mState;
        this->mBoard = other.mBoard;
        this->mScore = other.mScore;
    }

    Room(const Room &one)
    {
        clone(one);
    }

    Room &operator=(const Room &one)
    {
        if (this == &one)
            return *this;

        clone(one);
        return *this;
    }

    Room(Room &&) = default;
    Room &operator=(Room &&) = default;

    bool is_guest(std::shared_ptr<Player> p) const;
    bool is_player(std::shared_ptr<Player> p) const;
    bool is_counting_selecting(std::shared_ptr<Player> p) const;
    bool is_both_accept() const;
};

void to_json(nlohmann::json &j, const Room &r);
// void from_json(nlohmann::json& j, Room&);

#endif // ROOM_H