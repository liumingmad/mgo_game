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


struct InitClockTime
{
    int preTime;
    int readSecondCount;
    int moveTime;
};

class RClock
{
private:
    const InitClockTime Init_Time;

    // 保留时间
    int mPreTime;

    // 读秒次数
    int mReadSecondCount;

    // 每步棋的的剩余时间
    int mMoveTime;

public:
    RClock(InitClockTime time)
        : Init_Time(time),
            mPreTime(time.preTime),
            mReadSecondCount(time.readSecondCount),
            mMoveTime(time.moveTime)
    {
    }

    ~RClock()
    {
    }

    RClock(RClock &one) = delete;
    RClock &operator=(const RClock &one) = delete;
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

    // 减1，如果没时间了，则返回false，否则返回true
    bool subOne()
    {
        if (mPreTime > 0)
        {
            mPreTime--;
        }

        if (mMoveTime == 0)
        {
            mReadSecondCount--;
            mMoveTime = Init_Time.moveTime;
        }

        if (mReadSecondCount == 0)
        {
            return false;
        }
        return true;
    }
};

class GoClock
{
private:
    std::atomic<bool> mRunning = false;
    std::atomic<bool> mWaitingBlackMove = true;
    std::shared_ptr<RClock> mBClock;
    std::shared_ptr<RClock> mWClock;
    std::function<void(int)> mCallback;

public:
    GoClock(InitClockTime time) : mBClock(std::make_shared<RClock>(time)),
                                    mWClock(std::make_shared<RClock>(time))
    {
    }
    ~GoClock() {}

    void setCallback(std::function<void(int)> cb)
    {
        mCallback = cb;
    }

    void countdown()
    {
        TimerManager::instance().addTask(Timer::TIME_TASK_ID_COUNTDOWN, 1000, [&]()
                                            {
        if (!mRunning.load(std::memory_order_acquire)) return;

        if (mWaitingBlackMove.load(std::memory_order_acquire)) {
            mBClock->subOne();
        } else {
            mWClock->subOne();
        }

        if (mCallback) {
            mCallback(1);
        }

        countdown(); });
    }

    void start()
    {
        mRunning.store(true, std::memory_order_release);
        countdown();
    }

    void stop()
    {
        mRunning.store(false, std::memory_order_release);
    }

    void blackTap()
    {
        mWaitingBlackMove.store(false, std::memory_order_release);
    }

    void whiteTap()
    {
        mWaitingBlackMove.store(true, std::memory_order_release);
    }
};

enum RoomRole
{
    BLACK_PLAYER,
    WHITE_PLAYER,
    GUEST,
    UNKNOW
};

class Room
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

    // 当执行queue中Message的过程中，不能执行队列中下一个
    std::mutex mutex;
    SafeQueue<std::shared_ptr<Message>> queue;
    std::shared_ptr<RoomCore> core;

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

    Room(std::string id);
    ~Room();

    Room(const Room &one);
    Room &operator=(const Room &one);
    void clone(const Room &other);

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