#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>
#include <vector>

#include "body.h"
#include "Server.h"


// redis key prefix
const std::string KEY_USER_PREFIX = "user_id_";


// <user_id, Player>
extern std::map<std::string, Player> g_players;

extern std::map<std::string, Room> g_rooms;

// <fd, client>
extern std::map<int, std::shared_ptr<Client>> clientMap;
// <user_id, client>
extern std::map<std::string, std::shared_ptr<Client>> uidClientMap;


#endif // GLOBAL_H