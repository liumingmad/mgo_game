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

int RoomCore::run(std::shared_ptr<RoomMessage> roomMessage)
{
    std::shared_ptr<Message> msg = roomMessage->reqMsg;
    std::string action;
    if (roomMessage->action.empty())
    {
        action = msg->request->action;
    }
    else
    {
        action = roomMessage->action;
    }

    Log::info(action);

    if (action == "clock_tick") // 时钟滴答
    {
        do_clock_tick(roomMessage);
    }
    else if (action == "online")
    {
        do_online(roomMessage);
    }
    else if (action == "offline_timeout")
    {
        do_offline_timeout(roomMessage);
    }
    else if (action == "offline")
    {
        do_offline(roomMessage);
    }
    else if (action == "get_chat_list")
    {
        do_get_chat_list(roomMessage);
    }
    else if (action == "chat")
    {
        do_chat(roomMessage);
    }
    else if (action == "enter_room")
    {
        do_enter_room(roomMessage);
    }
    else if (action == "exit_room")
    {
        do_exit_room(roomMessage);
    }
    else if (action == "get_room_info")
    {
        do_get_room_info(roomMessage);
    }
    else if (action == "move")
    {
        do_waitting_move(roomMessage);
    }
    else if (action == "point_counting") // 申请点目
    {
        do_point_counting(roomMessage);
    }
    else if (action == "update_point_result") // 确认点目结果
    {
        do_point_counting_result(roomMessage);
    }
    else if (action == "gave_up") // 认输
    {
        do_gave_up(roomMessage);
    }
}

void RoomCore::do_clock_tick(std::shared_ptr<RoomMessage> msg)
{
    auto room = msg->room;
    std::shared_ptr<GoClock> goClock = std::any_cast<std::shared_ptr<GoClock>>(msg->data);
    std::shared_ptr<RClock> bc = goClock->getBClock();
    std::shared_ptr<RClock> wc = goClock->getWClock();

    // timeout
    bool blackTimeout = bc->getReadSecondCount() <= 0;
    bool whiteTimeout = wc->getReadSecondCount() <= 0;
    if (blackTimeout || whiteTimeout)
    {
        if (blackTimeout)
        {
            room->markBlackWins();
        }
        else
        {
            room->markWhiteWins();
        }

        std::string playerId;
        if (blackTimeout)
        {
            playerId = room->getBlackPlayer()->id;
        }
        else
        {
            playerId = room->getWhitePlayer()->id;
        }
        nlohmann::json j = {
            {"room_id", room->getId()},
            {"player_id", playerId},
        };
        std::shared_ptr<PushMessage> pmsg = std::make_shared<PushMessage>("move_timeout", j);
        room->pushMessageToAll(pmsg);
    }
    else
    {
        // push clock_tick
        nlohmann::json j = {
            {"room_id", room->getId()},
            {"b_clock", {{"preTime", bc->getPreTime()}, {"readSecondCount", bc->getReadSecondCount()}, {"moveTime", bc->getMoveTime()}}},
            {"w_clock", {{"preTime", wc->getPreTime()}, {"readSecondCount", wc->getReadSecondCount()}, {"moveTime", wc->getMoveTime()}}},
        };
        std::shared_ptr<PushMessage> pmsg = std::make_shared<PushMessage>("clock_tick", j);
        room->pushMessageToAll(pmsg);
    }
}

void RoomCore::do_enter_room(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::shared_ptr<Player> p = msg->self;

        RoomRole role = room->getRole(p);
        if (role != RoomRole::UNKNOW)
        {
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

void RoomCore::do_exit_room(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::shared_ptr<Player> p = msg->self;

        // 如果是棋手，不允许退出room, 只能通过认输退出
        RoomRole role = room->getRole(p);
        if (role != RoomRole::GUEST)
        {
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

void RoomCore::do_get_room_info(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        writeResponse(msg, Response{200, "success", *room.get()});
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

// 是否轮到user_id落子
bool should_move(const std::shared_ptr<Room> room, const std::string &user_id)
{
    bool should_move = false;

    int state = room->getState();
    bool is_waitting_black = state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
    bool is_waitting_white = state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;

    bool is_black = room->getBlackPlayer()->id == user_id;
    bool is_white = room->getWhitePlayer()->id == user_id;
    if (is_black)
    {
        should_move = is_waitting_black;
    }
    else if (is_white)
    {
        should_move = is_waitting_white;
    }
    return should_move;
}

void RoomCore::do_point_counting_result(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        // 1. 通过room id获取room
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::shared_ptr<Player> self = msg->self;
        bool is_accept = msg->request->data["state"];

        // 是否是下棋的人
        if (!room->is_player(self))
        {
            writeResponse(msg, Response{400, "You are not player, you are guest!", {}});
            return;
        }

        // 状态是否是正在选择
        if (!room->is_counting_selecting(self))
        {
            writeResponse(msg, Response{400, "is_counting_selecting false", {}});
            return;
        }

        // mark state
        if (self->color == "B")
        {
            room->switchPointCountingState(is_accept ? Room::COUNTING_STAT_BLACK_ACCEPT : Room::COUNTING_STAT_BLACK_REJECT);
        }
        else
        {
            room->switchPointCountingState(is_accept ? Room::COUNTING_STAT_WHITE_ACCEPT : Room::COUNTING_STAT_WHITE_REJECT);
        }

        // 操作成功
        writeResponse(msg, Response{200, "mark state success", {}});

        // 如果两个人都accept，则标记room状态, 推送game over到客户端
        if (room->is_both_accept())
        {
            room->switchRoomState(Room::ROOM_STATE_GAME_OVER);

            // 对于黑方和白方，则提示确认目数
            // 推送到room内所有人
            const std::map<std::string, std::shared_ptr<Player>> &map = room->getGuests();
            for (const auto &[uid, value] : map)
            {
                // ServerPusher::getInstance().server_push(uid, PushMessage{"point_counting_result", {
                //     {"room_id", room_id},
                //     {"score", room->getBoard().getScore()},
                // }});
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_point_counting(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        // 1. 通过room id获取room
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::string user_id = msg->self->id;

        // 2. 检查room状态，是否有人超时，或离线
        int state = room->getState();
        bool is_waitting_black = state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
        bool is_waitting_white = state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;
        if (!is_waitting_black && !is_waitting_white)
        {
            writeResponse(msg, Response{400, "move error, room state is " + room->getState(), {}});
            return;
        }

        // 3. 检查是否轮到当前用户落子
        if (!should_move(room, user_id))
        {
            writeResponse(msg, Response{400, "move error, your should be not move", {}});
            return;
        }

        // 暂停计时器
        // TimerManager::instance().removeTask(Timer::TIME_TASK_ID_WAITTING_MOVE);

        // switch state
        room->switchRoomState(Room::ROOM_STATE_POINT_COUNTTING);

        // 数目
        const Board &board = room->getBoard();
        Score score = board.computeScore();

        // 回复客户端200
        writeResponse(msg, Response{200, "point counting success, please waitting another accept", {}});

        // 推送到room内所有人
        // 对于黑方和白方，则提示确认目数
        const std::map<std::string, std::shared_ptr<Player>> &map = room->getGuests();
        for (const auto &[uid, value] : map)
        {
            // ServerPusher::getInstance().server_push(uid, PushMessage{"point_counting_result", {
            //     {"room_id", room_id},
            //     {"score", score},
            // }});
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_waitting_move(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::string user_id = msg->self->id;

        // 2. 检查room状态，是否有人超时，或离线
        int state = room->getState();
        bool is_waitting_black = state & Room::ROOM_STATE_WAITTING_BLACK_MOVE;
        bool is_waitting_white = state & Room::ROOM_STATE_WAITTING_WHITE_MOVE;
        if (!is_waitting_black && !is_waitting_white)
        {
            writeResponse(msg, Response{400, "move error, room state is ", {}});
            return;
        }

        // 3. 检查是否轮到当前用户落子
        if (!should_move(room, user_id))
        {
            writeResponse(msg, Response{400, "move error, your should be not move", {}});
            return;
        }

        // 4. 检查围棋规则
        Stone stone;
        const nlohmann::json &j = msg->request->data["stone"];
        from_json(j, stone);
        int success = room->getBoard().move(stone.x, stone.y, stone.color);
        if (!success)
        {
            writeResponse(msg, Response{400, "move error, move()", {}});
            return;
        }

        // 6. 回复客户端200
        writeResponse(msg, Response{200, "move success", {}});

        if (is_waitting_black)
        {
            room->getGoClock()->resumeWClock();
            room->switchRoomState(Room::ROOM_STATE_WAITTING_WHITE_MOVE);
        }
        else
        {
            room->getGoClock()->resumeBClock();
            room->switchRoomState(Room::ROOM_STATE_WAITTING_BLACK_MOVE);
        }

        room->pushMove(stone);
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_gave_up(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::string user_id = msg->self->id;

        // 检查是否轮到当前用户落子
        if (!should_move(room, user_id))
        {
            writeResponse(msg, Response{400, "move error, your should be not move", {}});
            return;
        }

        if (user_id == room->getBlackPlayer()->id)
        {
            room->markWhiteWins();
        }
        else if (user_id == room->getWhitePlayer()->id)
        {
            room->markBlackWins();
        }

        // push 给所有人
        room->pushGiveUp(user_id);
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

std::string getOfflineTaskId(std::string roomId)
{
    return "OFFLINE_TIMEOUT_" + roomId;
}

void RoomCore::do_online(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto room = roommsg->room;
        auto p = std::any_cast<std::shared_ptr<Player>>(roommsg->data);

        // p 是新创建的对象，所以要把room里保存的旧对象拷贝到新对象里
        // 调用赋值构造
        p = room->getBlackPlayer();
        p->state = PLAYER_ONLINE;

        // 取消超时检查
        std::string taskId = getOfflineTaskId(room->getId());
        TimerManager::instance().removeTask(taskId);

        // 打开go clock
        room->getGoClock()->start();

        // 通知room内其他人，自己上线
        room->pushOnline(p->id);
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_offline_timeout(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::string user_id = msg->self->id;

        if (user_id == room->getBlackPlayer()->id)
        {
            room->markBlackWins();
        }
        else if (user_id == room->getBlackPlayer()->id)
        {
            room->markWhiteWins();
        }
        room->pushGameResult();
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_offline(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::string user_id = msg->self->id;

        if (room->is_player(msg->self))
        {
            // mark offline
            msg->self->state = PLAYER_WAITTING_REBACK;

            // pause go clock
            room->getGoClock()->stop();

            // 开启定时器，如果180s，没回来，则判负
            std::string taskId = getOfflineTaskId(room->getId());
            TimerManager::instance().addTask(taskId, 180 * 1000, [&room, &user_id]()
                                             {
            // 得用发消息的方式，否则又并发了
            std::shared_ptr<RoomMessage> rmsg = std::make_shared<RoomMessage>();
            rmsg->room = room;
            rmsg->action = "offline_timeout";
            rmsg->data = user_id;
            room->postRoomMessage(rmsg); });
        }
        else
        {
            room->removeGuest(msg->self);
        }

        // 通知room内其他人，某人离线
        room->pushOffline(user_id);
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_chat(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;
        std::shared_ptr<Player> p = msg->self;
        std::string text = msg->request->data["text"].get<std::string>();

        auto one = std::make_shared<ChatMessage>(room->getId(), p->id, p->name, text);
        room->getChatMessages().push_back(one);

        writeResponse(msg, Response{200, "chat success", {}});
        // push
        room->pushChatMessage(one);
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}

void RoomCore::do_get_chat_list(std::shared_ptr<RoomMessage> roommsg)
{
    try
    {
        auto msg = roommsg->reqMsg;
        auto room = roommsg->room;

        Response resp;
        resp.code = 200;
        resp.message = "get_chat_list success";

        std::vector<ChatMessage> messages;
        for (auto one : room->getChatMessages()) {
            messages.push_back(*one);
        }
        resp.data = messages;

        writeResponse(msg, resp);
    }
    catch (const std::exception &e)
    {
        std::cerr << "其他错误: " << e.what() << std::endl;
    }
}