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

class RemainTime
{
private:
    // 保留时间
    const int Pre_Time;
    int mPreTime;

    // 读秒次数
    const int Read_Second_Count;
    int mReadSecondCount;

    // 每步棋的的剩余时间
    const int Move_Time;
    int mMoveTime;

public:
    RemainTime(int preTime, int readSecondCount, int moveTime)
        : Pre_Time(preTime),
          mPreTime(preTime),
          Read_Second_Count(readSecondCount),
          mReadSecondCount(readSecondCount),
          Move_Time(moveTime),
          mMoveTime(moveTime)
    {
    }

    ~RemainTime()
    {
    }

    RemainTime(RemainTime &one) = delete;
    RemainTime &operator=(const RemainTime &one) = delete;
    RemainTime(RemainTime&&) = default;
    RemainTime& operator=(RemainTime&&) = default;

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
            mMoveTime = Move_Time;
        }

        if (mReadSecondCount == 0)
        {
            return false;
        }
        return true;
    }
};

class Room
{
public:
    // 当执行queue中Message的过程中，不能执行队列中下一个
    std::mutex mutex;
    SafeQueue<std::shared_ptr<Message>> queue;
    std::shared_ptr<RoomCore> core;

    std::string id;
    std::map<std::string, std::shared_ptr<Player>> players;
    int state = ROOM_STATE_INIT;
    Board board;
    Score score;
    std::shared_ptr<RemainTime> mWhiteRemainTime;
    std::shared_ptr<RemainTime> mBlackRemainTime;

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
    uint8_t m_point_counting_state = COUNTING_STAT_INIT;


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

    Room() {}
    ~Room() {}
    /*
       std::string id;
    std::map<std::string, std::shared_ptr<Player>> players;
    int state = ROOM_STATE_INIT;
    Board board;
    Score score;
    std::shared_ptr<RemainTime> mWhiteRemainTime;
    std::shared_ptr<RemainTime> mBlackRemainTime; 
    */
    Room(const Room &one) {
        this->id = one.id;
        this->players = one.players;
        this->state = one.state;
        this->board = one.board;
        this->score = one.score;
    }
    Room &operator=(const Room &one) {
        if (this == &one) return *this;

        this->id = one.id;
        this->players = one.players;
        this->state = one.state;
        this->board = one.board;
        this->score = one.score;
        return *this;
    }
    Room(Room&&) = default;
    Room& operator=(Room&&) = default;

    bool is_guest(std::shared_ptr<Player> p) const;
    bool is_player(std::shared_ptr<Player> p) const;
    bool is_counting_selecting(std::shared_ptr<Player> p) const;
    bool is_both_accept() const;

    // 倒计时，每3秒调用一次
    void countdown();
};

void to_json(nlohmann::json &j, const Room &r);
// void from_json(nlohmann::json& j, Room&);

#endif // ROOM_H