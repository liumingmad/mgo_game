#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>
#include <vector>

#include "body.h"


std::map<std::string, Player> g_players;

std::map<std::string, Room> g_rooms;

// 服务器推送的序列号
long g_push_serial_number = 1;


#endif // GLOBAL_H