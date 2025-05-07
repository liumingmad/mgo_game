#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <string>

#include "Log.h"
#include "Handler.h"
#include "body.h"

class Core {
private:
    enum CoreState {
        UNAUTH,     // 未鉴权: sign_in 

        FREE,       // 鉴权通过: 
                    // get_room_list
                    // get_player_list
                    // get_sgf_list
                    // get_player_info
                    // create_room
                    // enter_room
                    // sign_out

        IN_ROOM,    // 创建房间后，进入这个状态。接受对局申请后，进入GAMING状态, 开始计时
                    // match 
                    // cancel_matching
                    // leave_room
                    // 

        GAMING,     // 游戏中(子状态机:GameState)
                    // admit_defeat
                    // move
                    // count_point

        FINISH,     // 游戏结束
    };

    CoreState m_state;
    std::string m_user_id;

public:
    Core(){
        m_state = UNAUTH;
    }

    ~Core(){}

    // 基于应答的状态机
    int run(std::shared_ptr<Message> msg);
    void handle_room_request(std::shared_ptr<Message> msg);

    void on_auth_success(const int fd, const std::string token);
    void do_sign_in(std::shared_ptr<Message> msg);
    void do_get_room_list(std::shared_ptr<Message> msg);
    void do_create_room(std::shared_ptr<Message> msg);
    void do_match_player(std::shared_ptr<Message> msg);
};

#endif // CORE_H