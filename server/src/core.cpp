#include "Log.h"
#include "core.h"
#include "json.hpp"
#include "body.h"
#include "cryptoUtils.h"
#include "http_utils.h"
#include <wrap.h>
#include "global.h"
#include "db_utils.h"
#include "protocol.h"


int writeResponse(Message& msg, Response response)
{
    nlohmann::json j = response;
    std::string json = j.dump();

    ProtocolWriter pw; 
    u_int8_t* buf = pw.wrap_header_buffer(msg.header->serial_number, json);
    Write(msg.fd, buf, HEADER_SIZE+json.length());
    return 0;
}

int do_sign_in(Message& msg, Request request)
{
    std::optional<User> user = check_token(request.data);
    if (!user)
    {
        return 0;
    }
    // 创建token
    std::string token = generate_jwt(std::to_string(user->id));
    writeResponse(msg, Response{200, "xxsign_in success", token});

    Log::info("Core::run() sign_in success");
    return 1;
}

int do_get_room_list(Message& msg, Request request)
{
    Response resp;
    resp.code = 200;
    resp.message = "get_room_list success";

    // array
    std::vector<Room> rooms;
    for (const auto &pair : g_rooms)
    {
        rooms.push_back(pair.second);
    }
    resp.data = rooms;
    writeResponse(msg, resp);
    return 0;
}

int do_create_room(Message& msg, Request request)
{
    Room room;
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    room.id = extract_user_id(request.token) + "_" + std::to_string(duration.count());
    g_rooms[room.id] = room;

    Response resp;
    resp.code = 200;
    resp.message = "create_room success";
    resp.data = room;
    writeResponse(msg, resp);
    return 0;
}

int do_enter_room(Message& msg, Request request)
{
    try{
        std::string room_id = request.data["room_id"].get<std::string>();
        Room room = g_rooms[room_id];
        std::string user_id = extract_user_id(request.token);
        Player* p = query_user(user_id);
        if (p) {
            room.players.push_back(*p);
            writeResponse(msg, Response{200, "enter_room success", room});
        } else {
            writeResponse(msg, Response{400, "enter_room failed", {}});
        }
    } catch (const std::exception& e) {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
    return 0;
}

int Core::run(Message &msg)
{
    Log::info("----------------------------");

    // 1. json转成对象
    Request request;
    try {
        nlohmann::json j = nlohmann::json::parse(msg.text);
        request = j.get<Request>();
        std::cout << "解析成功: " << j.dump(2) << std::endl;

        if (request.token.length() > 0) {
            std::string user_id = extract_user_id(request.token);
            std::cout << "core::run() user_id: " << user_id << std::endl;
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON 解析错误: " << e.what() << std::endl;
        std::cerr << "错误位置: 字节 " << e.byte << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }


    if (mState == UNAUTH)
    {
        // 断线重连进来, 暂时先默认进大厅
        if (request.token.length() > 0)
        {
            if (!validate_jwt(request.token))
            {
                Log::error("Core::run() token is invalid");
                writeResponse(msg, Response{401, "token is invalid", {}});
                return 0;
            }
            mState = FREE;
        } else {
            if (request.action == "sign_in")
            {
                do_sign_in(msg, request);
                mState = FREE;
                return 0;
            }
        }
    }

    if (mState == FREE)
    {
        if (!validate_jwt(request.token))
        {
            Log::error("Core::run() token is invalid");
            writeResponse(msg, Response{401, "token is invalid", {}});
            return 0;
        }

        if (request.action == "get_room_list")
        {
            do_get_room_list(msg, request);
        }
        else if (request.action == "create_room")
        {
            do_create_room(msg, request);
        }
        else if (request.action == "enter_room")
        {
            do_enter_room(msg, request);
        }
        else if (request.action == "get_online_player_list")
        {
        }
        else if (request.action == "find_player")
        {
        }
        else if (request.action == "get_player_info")
        {
        }
        else if (request.action == "get_sgf")
        {
        }
        else if (request.action == "get_sgf_list")
        {
        }
        else if (request.action == "sign_out")
        {
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