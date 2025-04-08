#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <string>

#include "Log.h"
#include "Handler.h"

class Core {
private:
    enum CoreState {
        UNAUTH,     // 未鉴权: sign_in 

        FREE,       // 鉴权通过: 
                    // get_room_list
                    // get_player_list
                    // get_sgf_list
                    // get_player_info
                    // create_room
                    // enter_room
                    // sign_out

        IN_ROOM,    // 创建房间后，进入这个状态。接受对局申请后，进入GAMING状态, 开始计时
                    // match 
                    // cancel_matching
                    // leave_room
                    // 

        GAMING,     // 游戏中(子状态机:GameState)
                    // admit_defeat
                    // move
                    // count_point

        FINISH,     // 游戏结束
    };

    enum GameState {
        PAUSE_OFFLINE,              // 计时器暂停, 对手掉线
        WAITTING_BLACK_MOVE,        // 计时器开启, 等待黑棋落子  
        WAITTING_WHITE_MOVE,        // 计时器开启, 等待白棋落子
        COUNTING_POINTS,            // 计时器暂停, 等待对局结束, 计算棋盘点数
        GAME_OVER,                  // 计时器暂停, 游戏结束
    };

    CoreState mState;
    GameState mGameState;
    std::string userId;

public:
    Core(){
        mState = UNAUTH;
    }

    ~Core(){}

    int run(Message& msg);
    void on_auth_success(std::string token);
};

#endif // CORE_H