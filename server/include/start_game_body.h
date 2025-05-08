#ifndef START_GAME_BODY_H
#define START_GAME_BODY_H

#include "room.h"

class StartGameBody
{
public:
    Room room;
    StartGameBody(Room r):room(r) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartGameBody, room)

#endif // START_GAME_BODY_H