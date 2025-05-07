#ifndef ROOM_CORE_H
#define ROOM_CORE_H

#include <iostream>
#include <string>

#include "Log.h"
#include "Handler.h"
#include "body.h"

class RoomCore {
public:
    RoomCore(){
    }

    ~RoomCore(){}

    int run(std::shared_ptr<Message> msg);
    void do_enter_room(std::shared_ptr<Message> msg);
    void do_exit_room(std::shared_ptr<Message> msg);
    void do_get_room_info(std::shared_ptr<Message> msg);
    void do_waitting_move(std::shared_ptr<Message> msg);
    void do_point_counting(std::shared_ptr<Message> msg);
    void do_point_counting_result(std::shared_ptr<Message> msg);
    void do_gave_up(std::shared_ptr<Message> msg);
};

#endif // ROOM_CORE_H