#ifndef DB_UTILS_H
#define DB_TUILS_H

#include "Sgf.h"
#include "player.h"

std::shared_ptr<Player> query_user(std::string id);

bool saveSgf(std::shared_ptr<Sgf> sgf);


#endif // DB_TUILS_H