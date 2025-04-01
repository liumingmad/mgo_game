#ifndef CORE_H
#define CORE_H

#include <iostream>

#include "Log.h"
#include "Handler.h"

class Core {
private:
    enum CoreState {
        UNAUTH,     // 未鉴权
        FREE,       // 鉴权通过
        INIT,       // 创建房间后，进入这个状态。接受对局申请后，进入GAMING状态, 开始计时
        GAMING,     // 游戏中(子状态机:GameState)
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

public:
    Core(){
        mState = UNAUTH;
    }

    ~Core(){}

    int run(Message& msg);
};

#endif // CORE_H