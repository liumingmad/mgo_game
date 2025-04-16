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

int writeResponse(Message &msg, Response response)
{
    nlohmann::json j = response;
    std::string json = j.dump();

    ProtocolWriter pw;
    u_int8_t *buf = pw.wrap_response_header_buffer(msg.header->serial_number, json);
    Write(msg.fd, buf, HEADER_SIZE + json.length());
    return 0;
}

void Core::do_sign_in(Message &msg, Request &request)
{
    try
    {
        std::string http_token = request.data["token"].get<std::string>();
        std::optional<User> user = check_token(http_token);
        if (!user)
        {
            writeResponse(msg, Response{400, "sign_in fail, token invalid", {}});
            return;
        }
        // 创建token
        SignInResponse resp;
        resp.token = generate_jwt(std::to_string(user->id));
        writeResponse(msg, Response{200, "sign_in success", resp});

        this->on_auth_success(msg.fd, resp.token);

        Log::info("Core::run() sign_in success");
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void Core::do_get_room_list(Message &msg, Request &request)
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
}

Room &create_room(std::string user_id)
{
    Room *room = new Room();
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    room->id = user_id + "_" + std::to_string(duration.count());
    return *room;
}

void Core::do_create_room(Message &msg, Request &request)
{
    Room &room = create_room(m_user_id);
    g_rooms[room.id] = room;

    Response resp;
    resp.code = 200;
    resp.message = "create_room success";
    resp.data = room;
    writeResponse(msg, resp);
}

void Core::do_enter_room(Message &msg, Request &request)
{
    try
    {
        std::string room_id = request.data["room_id"].get<std::string>();
        Room& room = g_rooms[room_id];
        std::string user_id = extract_user_id(request.token);
        Player *p = query_user(user_id);
        if (p)
        {
            room.players.push_back(*p);
            writeResponse(msg, Response{200, "enter_room success", room});
        }
        else
        {
            writeResponse(msg, Response{400, "enter_room failed", {}});
        }

        m_state = IN_ROOM;
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void Core::do_exit_room(Message &msg, Request &request)
{
    try
    {
        std::string room_id = request.data["room_id"].get<std::string>();
        std::string user_id = extract_user_id(request.token);

        std::vector<Player>& list = g_rooms[room_id].players;
        for (auto it=list.begin(); it != list.end(); it++) {
            if (it->id == user_id) {
                list.erase(it);
                break;
            }
        }

        writeResponse(msg, Response{200, "exit_room success", {}});
        m_state = FREE;
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void Core::do_get_room_info(Message &msg, Request &request)
{
    try
    {
        std::string room_id = request.data["room_id"].get<std::string>();
        Room room = g_rooms[room_id];
        writeResponse(msg, Response{200, "success", room});
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

// 0.客户端发送，匹配申请，服务端回复ok 最长等待30秒，客户端显示进度条,等待服务器推送
// 1.服务器先遍历申请人队列，如果有同级别的人，则匹配成功。
// 如果申请人队列没匹配成功，则遍历player列表, 向同级别的player申请对局
// 对方发送同意，则匹配成功
void Core::do_match_player(Message &msg, Request &request)
{
    try
    {
        writeResponse(msg, Response{200, "success, please waitting 30s", MatchPlayerResponse{AUTO_MATCH_DURATION}});

        // 0. 从缓存中找自己的Player
        Player &self = g_players[m_user_id];

        // 1. 找到对手, 从自动匹配队列中找
        AutoPlayerMatcher &matcher = AutoPlayerMatcher::getInstance();
        std::optional<Player> opponent = matcher.auto_match(self);
        if (!opponent.has_value())
        {
            matcher.enqueue_waitting(self);
            return;
        }

        // 2. 找到对手p后，创建room，把me和p加入到room
        int val = gen_random(0, 1);
        if (val == 1)
        {
            self.color = "B";
            opponent->color = "W";
        }
        else
        {
            self.color = "W";
            opponent->color = "B";
        }

        Room &room = create_room(m_user_id);
        room.players.push_back(self);
        room.players.push_back(*opponent);
        room.board.move(0, 1, 'B');
        g_rooms[room.id] = room;

        // 4. 开启对弈模式，切换状态机等
        m_state = GAMING;

        // 5. 给两个人分别推送一条消息, 客户端之间跳转到room page开始下棋
        ServerPusher &pusher = ServerPusher::getInstance();
        StartGameBody body;
        body.room = room;
        body.preTime = 5;
        body.readSecondCount = 3;
        body.moveTime = 60;
        pusher.server_push(msg.fd, PushMessage{"start_game", body});

        auto it = uidClientMap.find(opponent->id);
        if (it != uidClientMap.end())
        {
            pusher.server_push(it->second->fd, PushMessage{"start_game", body});
        }

        // 6. 推送后，客户端回复got it, 然后启动timer，切换到WAITTING_BLACK_MOVE
        m_game_state = WAITTING_BLACK_MOVE;
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void Core::on_auth_success(int fd, std::string token)
{
    Player p;

    std::string user_id = extract_user_id(token);
    const std::string key_user_id = KEY_USER_PREFIX + user_id;

    Redis &redis = RedisPool::getInstance().getRedis();
    auto player = redis.get(key_user_id);

    if (player.has_value())
    {
        nlohmann::json j = nlohmann::json::parse(*player);
        const Player &tmp = j.get<Player>();
        p = tmp;
    }
    else
    {
        Player* tmp = query_user(user_id);
        if (tmp == NULL)
        {
            std::cerr << "p == NULL" << std::endl;
            return;
        }
        p = *tmp;
        nlohmann::json j = p;
        redis.set(key_user_id, j.dump());
    }

    m_user_id = user_id;
    g_players.insert({p.id, p});

    std::shared_ptr<Client> client = clientMap[fd];
    client->user_id = p.id;
    uidClientMap.insert({p.id, client});

    AsyncEventBus::getInstance().asyncPublish<std::string>(EventHandler::EVENT_ONLINE, user_id);
}

int Core::run(Message &msg)
{
    Log::info("\n\n----------------------------");

    // 1. json转成对象
    Request request;
    try
    {
        nlohmann::json j = nlohmann::json::parse(msg.text);
        request = j.get<Request>();
        std::cout << m_state << "解析成功: " << j.dump(2) << std::endl;

        if (request.token.length() > 0)
        {
            std::string user_id = extract_user_id(request.token);
            std::cout << "core::run() user_id: " << user_id << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "JSON 解析错误: " << e.what() << std::endl;
        std::cerr << "错误位置: 字节 " << e.byte << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }

    if (m_state == UNAUTH)
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
            on_auth_success(msg.fd, request.token);
            m_state = FREE;
        }
        else
        {
            if (request.action == "sign_in")
            {
                do_sign_in(msg, request);
                m_state = FREE;
                return 0;
            }
        }
    }

    if (m_state == FREE)
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
        else if (request.action == "get_room_info")
        {
            do_get_room_info(msg, request);
        }
        else if (request.action == "match_player")
        {
            do_match_player(msg, request);
        }
        else if (request.action == "cancel_matching")
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
    else if (m_state == IN_ROOM) // 观战状态
    {
        if (!validate_jwt(request.token))
        {
            Log::error("Core::run() token is invalid");
            writeResponse(msg, Response{401, "token is invalid", {}});
            return 0;
        }

        if (request.action == "get_room_info")
        {
            do_get_room_info(msg, request);
        }
        else if (request.action == "exit_room")
        {
            do_exit_room(msg, request);
        }
    }
    else if (m_state == GAMING) // 下棋中
    {
        gaming_run(msg, request);
    }
    else if (m_state == FINISH)
    {
    }
    return 0;
}

// 客户端发的消息，落子/点目申请/认输
int Core::gaming_run(Message &msg, Request& request)
{
    if (request.action == "get_room_info")
    {
        do_get_room_info(msg, request);
    }

    if (m_game_state == WAITTING_BLACK_MOVE)
    {
        // 1. 通过room id获取room
        // 2. 检查room状态，是否有人超时，或离线
        // 3. 检查当前用户是否是执黑的player
        // 4. 检查围棋规则
        // 5. 推送落子到room内所有人
    }
    else if (m_game_state == WAITTING_WHITE_MOVE)
    {
    }
    else if (m_game_state == COUNTING_POINTS)
    {
    }
    else if (m_game_state == GAME_OVER)
    {
    }
}