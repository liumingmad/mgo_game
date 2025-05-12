#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <string>

#include "Log.h"
#include "message.h"
#include "body.h"

class Core {
public:
    Core(){}

    ~Core(){}

    // 基于应答的状态机
    int run(std::shared_ptr<Message> msg);

    void findRoomListByPlayer(std::vector<std::shared_ptr<Room>>& list, 
        std::vector<std::shared_ptr<Room>>& guestList, std::shared_ptr<Player> p);
    std::shared_ptr<Room> getNeedBackRoom(std::shared_ptr<Player> p);

    void do_online(std::shared_ptr<Message> msg);
    void do_offline(std::shared_ptr<Message> msg);

    bool checkAuth(std::shared_ptr<Message> msg);
    void on_auth_success(const int fd, const std::string token);
    void do_sign_in(std::shared_ptr<Message> msg);
    void do_get_room_list(std::shared_ptr<Message> msg);
    void do_create_room(std::shared_ptr<Message> msg);
    void do_match_player(std::shared_ptr<Message> msg);
};

#endif // CORE_H