#include "Log.h"
#include "core.h"
#include "json.hpp"
#include "body.h"
#include "cryptoUtils.h"
#include "http_utils.h"
#include <wrap.h>

int writeResponse(int fd, Response response)
{
    nlohmann::json j = response;
    std::string json = j.dump();
    const char *text = json.c_str();
    Write(fd, text, json.length());
    return 0;
}

int do_sign_in(int fd, Request request)
{
    std::optional<User> user = check_token(request.data);
    if (!user)
    {
        return 0;
    }
    // 创建token
    std::string token = generate_jwt("" + user->id);
    Response resp;
    resp.action = "sign_in";
    resp.data = nlohmann::json{{"token", token}}.dump();
    writeResponse(fd, resp);
    Log::info("Core::run() sign_in success");
    return 1;
}

int Core::run(Message &msg)
{
    Log::info("Core::run()");

    // 1. json转成对象
    nlohmann::json j = nlohmann::json::parse(msg.text);
    Request request = j.get<Request>();

    // 2. 根据状态机处理消息
    if (mState == UNAUTH)
    {
        if (request.action == "sign_in")
        {
            do_sign_in(msg.fd, request);
            mState = FREE;
        }
    }
    else if (mState == FREE)
    {
        if (request.action == "get_room_list") {

        } else if (request.action == "get_online_player_list") {

        } else if (request.action == "find_player") {

        } else if (request.action == "get_player_info") {

        } else if (request.action == "get_sgf") {

        } else if (request.action == "get_sgf_list") {

        } else if (request.action == "create_room") {

        } else if (request.action == "enter_room") {

        } else if (request.action == "sign_out") {

        }
    }
    else if (mState == IN_ROOM)
    {
        // match
        // cancel_matching
        // leave_room
    }
    else if (mState == GAMING)
    {
        // admit_defeat
        // move
        // count_point

        if (mGameState == PAUSE_OFFLINE)
        {
        }
        else if (mGameState == WAITTING_BLACK_MOVE)
        {
        }
        else if (mGameState == WAITTING_WHITE_MOVE)
        {
        }
        else if (mGameState == COUNTING_POINTS)
        {
        }
        else if (mGameState == GAME_OVER)
        {
        }
    }
    else if (mState == FINISH)
    {
    }
    return 0;
}