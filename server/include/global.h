#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>
#include <vector>

#include "body.h"
#include "Server.h"
#include "room.h"
#include "event_bus.h"
#include "ThreadSafeUnorderedMap.h"


// redis key prefix
const std::string KEY_USER_PREFIX = "user_id_";

// 30s
const int AUTO_MATCH_DURATION = 30 * 200;

// <user_id, Player>
extern ThreadSafeUnorderedMap<std::string, std::shared_ptr<Player>> g_players;

extern ThreadSafeUnorderedMap<std::string, std::shared_ptr<Room>> g_rooms;

// <fd, client>
extern ThreadSafeUnorderedMap<int, std::shared_ptr<Client>> g_clientMap;
// <client_id, client>
extern ThreadSafeUnorderedMap<int, std::shared_ptr<Client>> g_clientIdMap;
// <user_id, client>
extern ThreadSafeUnorderedMap<std::string, std::shared_ptr<Client>> g_uidClientMap;

extern ThreadPool g_room_pool;

extern EventBus g_EventBus;




#endif // GLOBAL_H