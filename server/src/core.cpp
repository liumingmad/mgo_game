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
#include "auto_match.h"
#include <sw/redis++/redis++.h>
#include <redis_pool.h>
#include <push_message.h>
#include <server_push.h>
#include "Stone.h"
#include "player.h"
#include "common_utils.h"

void Core::do_sign_in(std::shared_ptr<Message> msg)
{
    try
    {
        std::string http_token = msg->request->data["token"].get<std::string>();
        std::optional<User> user = check_token(http_token);
        if (!user)
        {
            writeResponse(msg, Response{400, "sign_in fail, token invalid", {}});
            Log::info("Core::run() sign_in fail, token invalid");
            return;
        }
        // 创建token
        SignInResponse resp;
        resp.token = generate_jwt(std::to_string(user->id));
        writeResponse(msg, Response{200, "sign_in success", resp});

        this->on_auth_success(msg->fd, resp.token);

        Log::info("Core::run() sign_in success");
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void Core::do_get_room_list(std::shared_ptr<Message> msg)
{
    Response resp;
    resp.code = 200;
    resp.message = "get_room_list success";

    // array
    std::vector<Room> rooms;
    g_rooms.for_each([&rooms](const auto& pair) {
        rooms.push_back(*(pair.second));
    });
    resp.data = rooms;

    writeResponse(msg, resp);
}

std::shared_ptr<Room> create_room(const std::string user_id)
{
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::string id = user_id + "_" + std::to_string(duration.count());
    std::shared_ptr<Room> room = Room::create(id);
    return room;
}

void Core::do_create_room(std::shared_ptr<Message> msg)
{
    std::shared_ptr<Room> room = create_room(m_user_id);
    g_rooms.set(room->getId(), room);

    Response resp;
    resp.code = 200;
    resp.message = "create_room success";
    resp.data = *(room.get());
    writeResponse(msg, resp);
}

void Core::on_auth_success(int fd, std::string token)
{
    std::shared_ptr<Player> p;

    std::string user_id = extract_user_id(token);
    const std::string key_user_id = KEY_USER_PREFIX + user_id;

    Redis &redis = RedisPool::getInstance().getRedis();
    auto player = redis.get(key_user_id);

    if (player.has_value())
    {
        nlohmann::json j = nlohmann::json::parse(*player);
        p = std::make_shared<Player>(j.get<Player>());
    }
    else
    {
        std::shared_ptr<Player> tmp = query_user(user_id);
        if (tmp == NULL)
        {
            std::cerr << "p == NULL" << std::endl;
            return;
        }
        p = tmp;
        nlohmann::json j = *p;
        redis.set(key_user_id, j.dump());
    }

    m_user_id = user_id;

    auto client = g_clientMap.get(fd);
    if (client.has_value()) {
        client.value()->user_id = p->id;
        g_uidClientMap.set(p->id, client.value());
    }
    g_players.set(p->id, p);

    // AsyncEventBus::getInstance().asyncPublish<std::string>(EventHandler::EVENT_ONLINE, user_id);
}

// 0.客户端发送，匹配申请，服务端回复ok 最长等待30秒，客户端显示进度条,等待服务器推送
// 1.服务器先遍历申请人队列，如果有同级别的人，则匹配成功。
// 如果申请人队列没匹配成功，则遍历player列表, 向同级别的player申请对局
// 对方发送同意，则匹配成功
void Core::do_match_player(std::shared_ptr<Message> msg)
{
    try
    {
        writeResponse(msg, Response{200, "success, please waitting 30s", MatchPlayerResponse{AUTO_MATCH_DURATION}});

        if (false) {
            std::shared_ptr<Room> room = create_room(m_user_id);
            g_rooms.set(room->getId(), room);
            InitClockTime time;
            time.preTime = 1;
            time.readSecondCount = 1;
            time.moveTime = 10;
            room->createGoClock(time);
            room->getGoClock()->start();
            return;
        }

        // 0. 从缓存中找自己的Player
        auto opt = g_players.get(m_user_id);
        std::shared_ptr<Player> self = opt.value();

        // 1. 找到对手, 从自动匹配队列中找
        AutoPlayerMatcher &matcher = AutoPlayerMatcher::getInstance();
        std::shared_ptr<Player> opponent = matcher.auto_match(*self);
        if (!opponent)
        {
            matcher.enqueue_waitting(self);
            return;
        }

        // 2. 找到对手p后，创建room，把me和p加入到room
        std::shared_ptr<Room> room = create_room(m_user_id);

        int val = gen_random(0, 1);
        if (val == 1)
        {
            self->color = "B";
            opponent->color = "W";
            room->setBlackPlayer(self);
            room->setWhitePlayer(opponent);
        }
        else
        {
            self->color = "W";
            opponent->color = "B";
            room->setBlackPlayer(opponent);
            room->setWhitePlayer(self);
        }

        MatchPlayerRequest req;
        const nlohmann::json& j = msg->request->data;
        from_json(j, req);
        InitClockTime time;
        time.preTime = req.preTime;
        time.readSecondCount = req.readSecondCount;
        time.moveTime = req.moveTime;
        room->createGoClock(time);
        room->getGoClock()->start();

        g_rooms.set(room->getId(), room);
        room->switchRoomState(Room::ROOM_STATE_WAITTING_BLACK_MOVE);

        room->pushStartGame();

    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
        print_stacktrace();
    }
}

int Core::run(std::shared_ptr<Message> msg)
{
    std::string user_id;
    if (msg->request->token.length() == 0)
    {
        if (msg->request->action == "sign_in")
        {
            do_sign_in(msg);
        } else {
            writeResponse(msg, Response{401, "token is invalid", {}});
        }
        return 0;
    } 

    user_id = extract_user_id(msg->request->token);
    std::cout << "core::run() user_id: " << user_id << std::endl;
    if (!validate_jwt(msg->request->token))
    {
        Log::error("Core::run() token is invalid");
        writeResponse(msg, Response{401, "token is invalid", {}});
        return 0;
    }
    on_auth_success(msg->fd, msg->request->token);


    if (msg->request->action == "get_room_list")
    {
        do_get_room_list(msg);
    }
    else if (msg->request->action == "create_room")
    {
        do_create_room(msg);
    }
    else if (msg->request->action == "match_player")
    {
        do_match_player(msg);
    }
    else if (msg->request->action == "cancel_matching")
    {
    }
    else if (msg->request->action == "find_player")
    {
    }
    else if (msg->request->action == "get_player_info")
    {
    }
    else if (msg->request->action == "get_sgf")
    {
    }
    else if (msg->request->action == "get_sgf_list")
    {
    }
    else if (msg->request->action == "sign_out")
    {
    }


    // 同一个room内的操作，单线程处理
    else if (msg->request->action == "enter_room"
        || msg->request->action == "exit_room"
        || msg->request->action == "get_room_info"
        || msg->request->action == "move"
        || msg->request->action == "point_counting"
        || msg->request->action == "update_point_result"
        || msg->request->action == "gave_up"
    )
    {
        std::string room_id = msg->request->data["room_id"].get<std::string>();
        std::shared_ptr<Room> room = g_rooms.get(room_id).value();

        std::shared_ptr<RoomMessage> rmsg = std::make_shared<RoomMessage>();
        rmsg->reqMsg = msg;
        room->postRoomMessage(rmsg);
    }

    return 0;
}


