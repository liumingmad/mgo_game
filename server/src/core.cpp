#include "Log.h"
#include "core.h"
#include "json.hpp"
#include "body.h"
#include "cryptoUtils.h"
int Core::run(Message& msg)
{
    Log::info("Core::run()");

    // 1. json转成对象
    nlohmann::json j = nlohmann::json::parse(msg.text);
    Request request = j.get<Request>();

    // 2. 根据状态机处理消息
    if (mState == UNAUTH) {
        std::string token = generate_jwt("123");
        std::cout << token << std::endl;

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