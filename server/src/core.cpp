#include "Log.h"
#include "core.h"
int Core::run(Message& msg)
{
    Log::info("Core::run()");

    // 1. json转成对象

    // 2. 根据状态机处理消息

    if (mState == UNAUTH) {

    } else if (mState == FREE) {

    } else if (mState == INIT) {

    } else if (mState == GAMING) {

        if (mGameState == PAUSE_OFFLINE) {

        } else if (mGameState == WAITTING_BLACK_MOVE) {

        } else if (mGameState == WAITTING_WHITE_MOVE) {

        } else if (mGameState == COUNTING_POINTS) {

        } else if (mGameState == GAME_OVER) {

        }

    } else if (mState == FINISH) {

    }
    return 0;
}