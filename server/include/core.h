#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <string>

#include "Log.h"
#include "message.h"
#include "body.h"

class Core {
private:
    std::string m_user_id;

public:
    Core(){}

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