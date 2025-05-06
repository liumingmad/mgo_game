#ifndef ROOM_H
#define ROOM_H


#include <string>
#include <map>
#include "body.h"
#include "json.hpp"
#include "Board.h"
#include "Node.h"


class Room {
public:
    std::string id;
    std::map<std::string, std::shared_ptr<Player>> players;
    int state = ROOM_STATE_INIT;
    Board board;

    Score score;

    // 数目的状态
    // 每个人有三种状态：argee/reject/selecting
    // 用6个二进制位表示
    static const u_int8_t COUNTING_STAT_INIT               =            0;
    static const u_int8_t COUNTING_STAT_BLACK_ACCEPT       =            1;
    static const u_int8_t COUNTING_STAT_BLACK_REJECT      =            1 << 1;
    static const u_int8_t COUNTING_STAT_BLACK_SELECTING   =            1 << 2;
    static const u_int8_t COUNTING_STAT_WHITE_ACCEPT      =            1 << 3;
    static const u_int8_t COUNTING_STAT_WHITE_REJECT      =            1 << 4;
    static const u_int8_t COUNTING_STAT_WHITE_SELECTING   =            1 << 5;
    uint8_t m_point_counting_state = COUNTING_STAT_INIT;

    int preTime; // 5*60 s
    int moveTime; // 60 seconds
    int readSecondCount; // 读秒次数

    int remain_pretime;
    int remain_movetime;
    int remain_read_second_count;

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

    Room(){}
    ~Room(){}

    bool is_guest(std::shared_ptr<Player> p) const;
    bool is_player(std::shared_ptr<Player> p) const;
    bool is_counting_selecting(std::shared_ptr<Player> p) const;
    bool is_both_accept() const;

};

void to_json(nlohmann::json& j, const Room& r);
// void from_json(nlohmann::json& j, Room&);


#endif // ROOM_H