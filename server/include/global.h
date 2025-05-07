#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>
#include <vector>

#include "body.h"
#include "Server.h"
#include "room.h"


// redis key prefix
const std::string KEY_USER_PREFIX = "user_id_";

// 30s
const int AUTO_MATCH_DURATION = 30 * 200;

// <user_id, Player>
extern std::map<std::string, std::shared_ptr<Player>> g_players;

extern std::map<std::string, std::shared_ptr<Room>> g_rooms;

// <fd, client>
extern std::map<int, std::shared_ptr<Client>> g_clientMap;
// <user_id, client>
extern std::map<std::string, std::shared_ptr<Client>> g_uidClientMap;

extern ThreadPool g_room_pool;


#endif // GLOBAL_H