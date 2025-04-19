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
            room.players[p->id] = *p;
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
        g_rooms[room_id].players.erase(user_id);
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
        room.players[self.id] = self;
        room.players[opponent->id] = *opponent;
        room.state = Room::ROOM_STATE_WAITTING_BLACK_MOVE;
        g_rooms[room.id] = room;

        // 4. 开启对弈模式，切换状态机等
        m_state = GAMING;

        // 5. 给两个人分别推送一条消息, 客户端之间跳转到room page开始下棋
        ServerPusher &pusher = ServerPusher::getInstance();
        StartGameBody body;
        body.room = room;
        pusher.server_push(msg.fd, PushMessage{"start_game", body});

        auto it = uidClientMap.find(opponent->id);
        if (it != uidClientMap.end())
        {
            pusher.server_push(it->second->fd, PushMessage{"start_game", body});
        }

        m_game_state = WAITTING_MOVE;

        // 开定时器，等待30秒，第一步
        int fd = msg.fd;
        std::string room_id = room.id;
        TimerManager::instance().addTask(Timer::TIME_TASK_ID_WAITTING_MOVE, 30000, [fd, room_id](){
            std::cout << "invalid game" << std::endl; 
            // 1. push message: invalid game
            ServerPusher::getInstance().server_push(fd, PushMessage{"first_move_timeout", {}});

            // 2. set room state
            Room& room = g_rooms[room_id];
            room.state = Room::ROOM_STATE_GAME_OVER;
        });
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

    Request request;
    try
    {
        nlohmann::json j = nlohmann::json::parse(msg.text);
        request = j.get<Request>();
        std::cout << m_state << "解析成功: " << j.dump(2) << std::endl;
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

    std::string user_id;
    if (request.token.length() == 0)
    {
        if (request.action == "sign_in")
        {
            do_sign_in(msg, request);
        } else {
            writeResponse(msg, Response{401, "token is invalid", {}});
        }
        return 0;
    } 

    user_id = extract_user_id(request.token);
    std::cout << "core::run() user_id: " << user_id << std::endl;
    if (!validate_jwt(request.token))
    {
        Log::error("Core::run() token is invalid");
        writeResponse(msg, Response{401, "token is invalid", {}});
        return 0;
    }
    on_auth_success(msg.fd, request.token);


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
    else if (request.action == "get_room_info")
    {
        do_get_room_info(msg, request);
    }
    else if (request.action == "exit_room")
    {
        do_exit_room(msg, request);
    }
    else if (request.action == "get_room_info")
    {
        do_get_room_info(msg, request);
    } 
    else if (request.action == "move") 
    {
        do_waitting_move(msg, request);
    }
    else if (request.action == "point_counting") 
    {
    }
    return 0;
}

void Core::do_waitting_move(Message &msg, Request &request) {
    // 1. 通过room id获取room
    std::string room_id = request.data["room_id"].get<std::string>();
    Room& room = g_rooms[room_id];
    std::string user_id = extract_user_id(request.token);

    // 2. 检查room状态，是否有人超时，或离线
    bool is_waitting_black = room.state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
    bool is_waitting_white = room.state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;
    if (!is_waitting_black && !is_waitting_white) {
        writeResponse(msg, Response{400, "move error, room state is "+room.state, {}});
        return;
    } 

    // 3. 检查是否轮到当前用户落子
    bool should_move = false;
    const Player& p = room.players[user_id]; 
    bool self_is_black = p.color == "B";
    if (self_is_black) {
        should_move = is_waitting_black;
    } else {
        should_move = is_waitting_white;
    }

    if (!should_move) {
        writeResponse(msg, Response{400, "move error, your should be not move", {}});
        return;
    }

    // 4. 检查围棋规则
    Stone stone;
    const nlohmann::json& j = request.data["stone"];
    from_json(j, stone);
    int success = room.board.move(stone.x, stone.y, stone.color);
    if (!success) {
        writeResponse(msg, Response{400, "move error, move()", {}});
        return;
    }

    // 5. 推送落子到room内所有人
    std::map<std::string, Player>& map = room.players;
    for (const auto& [key, value] : map) {
        const int fd = uidClientMap[key]->fd;
        if (fd == msg.fd) continue;
        ServerPusher::getInstance().server_push(fd, PushMessage{"move", {
            {"room_id", room_id},
            {"stone", stone},
        }});
    }

    // 6. 回复客户端200
    writeResponse(msg, Response{200, "move success", {}});
    
    // 7. 添加定时器，如果对手超时没落子，就判负
    TimerManager::instance().addTask(Timer::TIME_TASK_ID_WAITTING_MOVE, 30000, [room_id](){
        // push message for all
        Room& room = g_rooms[room_id];
        std::map<std::string, Player>& map = room.players;
        for (const auto& [key, value] : map) {
            const int fd = uidClientMap[key]->fd;
            ServerPusher::getInstance().server_push(fd, PushMessage{"move_timeout", {
                {"room_id", room_id},
            }});
        }
    });
}