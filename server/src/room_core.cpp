#include "Log.h"
#include "json.hpp"
#include "body.h"
#include "cryptoUtils.h"
#include "http_utils.h"
#include "wrap.h"
#include "global.h"
#include "protocol.h"
#include "auto_match.h"
#include <sw/redis++/redis++.h>
#include <redis_pool.h>
#include "push_message.h"
#include "server_push.h"
#include "Stone.h"
#include "room_core.h"
#include "room.h"
#include "player.h"


int RoomCore::run(std::shared_ptr<Message> msg) {
    if (msg->request->action == "enter_room")
    {
        do_enter_room(msg);
    }
    else if (msg->request->action == "exit_room")
    {
        do_exit_room(msg);
    }
    else if (msg->request->action == "get_room_info")
    {
        do_get_room_info(msg);
    } 
    else if (msg->request->action == "move") 
    {
        do_waitting_move(msg);
    }
    else if (msg->request->action == "point_counting")  // 申请点目
    {
        do_point_counting(msg);
    }
    else if (msg->request->action == "update_point_result") // 确认点目结果
    {
        do_point_counting_result(msg);
    }
    else if (msg->request->action == "gave_up")  // 认输
    {
        do_gave_up(msg);
    }
}

void RoomCore::do_enter_room(std::shared_ptr<Message> msg)
{
    try
    {
        std::string room_id = msg->request->data["room_id"].get<std::string>();
        std::shared_ptr<Room> room = g_rooms[room_id];

        std::string user_id = extract_user_id(msg->request->token);
        auto it = g_players.find(user_id);
        if (it == g_players.end()) {
            writeResponse(msg, Response{400, "find player failed", {}});
            return;
        }

        std::shared_ptr<Player> p = it->second;
        RoomRole role = room->getRole(p);
        if (role != RoomRole::UNKNOW) {
            writeResponse(msg, Response{200, "role rejected", *room.get()});
            return;
        } 

        room->addGuest(p);
        writeResponse(msg, Response{200, "enter_room success", *room.get()});
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_exit_room(std::shared_ptr<Message> msg)
{
    try
    {
        std::string room_id = msg->request->data["room_id"].get<std::string>();
        std::shared_ptr<Room> room = g_rooms[room_id];

        std::string user_id = extract_user_id(msg->request->token);
        auto it = g_players.find(user_id);
        if (it == g_players.end()) {
            writeResponse(msg, Response{400, "find player failed", {}});
            return;
        }

        // 如果是棋手，不允许退出room, 只能通过认输退出
        std::shared_ptr<Player> p = it->second;
        RoomRole role = room->getRole(p);
        if (role != RoomRole::GUEST) {
            writeResponse(msg, Response{200, "exit_room role is not guest", {}});
            return;
        }

        room->removeGuest(p);
        writeResponse(msg, Response{200, "exit_room success", {}});
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_get_room_info(std::shared_ptr<Message> msg)
{
    try
    {
        std::string room_id = msg->request->data["room_id"].get<std::string>();
        std::shared_ptr<Room> room = g_rooms[room_id];
        writeResponse(msg, Response{200, "success", *room.get()});
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

// 是否轮到user_id落子
bool should_move(const std::shared_ptr<Room> room, const std::string& user_id) {
    bool should_move = false;

    int state = room->getState();
    bool is_waitting_black = state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
    bool is_waitting_white = state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;

    bool is_black = room->getBlackPlayer()->id == user_id;
    bool is_white = room->getWhitePlayer()->id == user_id;
    if (is_black) {
        should_move = is_waitting_black;
    } else if (is_white) {
        should_move = is_waitting_white;
    }
    return should_move;
}

void RoomCore::do_gave_up(std::shared_ptr<Message> msg) {

}

void RoomCore::do_point_counting_result(std::shared_ptr<Message> msg) {
    // 1. 通过room id获取room
    std::string room_id = msg->request->data["room_id"].get<std::string>();
    std::shared_ptr<Room> room = g_rooms[room_id];

    std::string user_id = extract_user_id(msg->request->token);
    std::shared_ptr<Player> self = g_players[user_id]; 

    bool is_accept = msg->request->data["state"];

    // 是否是下棋的人
    if (!room->is_player(self)) {
        writeResponse(msg, Response{400, "You are not player, you are guest!", {}});
        return;
    } 

    // 状态是否是正在选择
    if (!room->is_counting_selecting(self)) {
        writeResponse(msg, Response{400, "is_counting_selecting false", {}});
        return;
    }

    // mark state
    if (self->color == "B") {
        room->switchPointCountingState(is_accept ? Room::COUNTING_STAT_BLACK_ACCEPT : Room::COUNTING_STAT_BLACK_REJECT);
    } else {
        room->switchPointCountingState(is_accept ? Room::COUNTING_STAT_WHITE_ACCEPT : Room::COUNTING_STAT_WHITE_REJECT);
    }

    // 操作成功
    writeResponse(msg, Response{200, "mark state success", {}});

    // 如果两个人都accept，则标记room状态, 推送game over到客户端
    if (room->is_both_accept()) {
        room->switchRoomState(Room::ROOM_STATE_GAME_OVER);

        // 对于黑方和白方，则提示确认目数
        // 推送到room内所有人
        const std::map<std::string, std::shared_ptr<Player>>& map = room->getGuests();
        for (const auto& [uid, value] : map) {
            // ServerPusher::getInstance().server_push(uid, PushMessage{"point_counting_result", {
            //     {"room_id", room_id},
            //     {"score", room->getBoard().getScore()},
            // }});
        }
    }
}

void RoomCore::do_point_counting(std::shared_ptr<Message> msg) {
    // 1. 通过room id获取room
    std::string room_id = msg->request->data["room_id"].get<std::string>();
    std::shared_ptr<Room> room = g_rooms[room_id];
    std::string user_id = extract_user_id(msg->request->token);

    // 2. 检查room状态，是否有人超时，或离线
    int state = room->getState();
    bool is_waitting_black = state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
    bool is_waitting_white = state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;
    if (!is_waitting_black && !is_waitting_white) {
        writeResponse(msg, Response{400, "move error, room state is "+room->getState(), {}});
        return;
    } 

    // 3. 检查是否轮到当前用户落子
    if (!should_move(room, user_id)) {
        writeResponse(msg, Response{400, "move error, your should be not move", {}});
        return;
    }

    // 暂停计时器
    TimerManager::instance().removeTask(Timer::TIME_TASK_ID_WAITTING_MOVE);

    // switch state
    room->switchRoomState(Room::ROOM_STATE_POINT_COUNTTING);

    // 数目
    const Board& board = room->getBoard();
    Score score = board.computeScore();

    // 回复客户端200
    writeResponse(msg, Response{200, "point counting success, please waitting another accept", {}});

    // 推送到room内所有人
    // 对于黑方和白方，则提示确认目数
    const std::map<std::string, std::shared_ptr<Player>>& map = room->getGuests();
    for (const auto& [uid, value] : map) {
        // ServerPusher::getInstance().server_push(uid, PushMessage{"point_counting_result", {
        //     {"room_id", room_id},
        //     {"score", score},
        // }});
    }
}

void RoomCore::do_waitting_move(std::shared_ptr<Message> msg) {
    // 1. 通过room id获取room
    std::string room_id = msg->request->data["room_id"].get<std::string>();
    std::shared_ptr<Room> room = g_rooms[room_id];
    std::string user_id = extract_user_id(msg->request->token);

    // 2. 检查room状态，是否有人超时，或离线
    int state = room->getState();
    bool is_waitting_black = state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
    bool is_waitting_white = state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;
    if (!is_waitting_black && !is_waitting_white) {
        writeResponse(msg, Response{400, "move error, room state is "+room->getState(), {}});
        return;
    } 

    // 3. 检查是否轮到当前用户落子
    if (!should_move(room, user_id)) {
        writeResponse(msg, Response{400, "move error, your should be not move", {}});
        return;
    }

    // 4. 检查围棋规则
    Stone stone;
    const nlohmann::json& j = msg->request->data["stone"];
    from_json(j, stone);
    int success = room->getBoard().move(stone.x, stone.y, stone.color);
    if (!success) {
        writeResponse(msg, Response{400, "move error, move()", {}});
        return;
    }

    // 6. 回复客户端200
    writeResponse(msg, Response{200, "move success", {}});

    if (is_waitting_black) {
        room->switchRoomState(Room::ROOM_STATE_WAITTING_WHITE_MOVE);
    } else {
        room->switchRoomState(Room::ROOM_STATE_WAITTING_BLACK_MOVE);
    }


}