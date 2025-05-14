#ifndef SGF_H
#define SGF_H

#include <string>
#include "json.hpp"

struct Sgf
{
    std::string bName;
    std::string wName;
    int bLevel;
    int wLevel;
    std::string sgf;
    long timeMs;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sgf, bName, wName, bLevel, wLevel, sgf, timeMs)


#endif // SGF_H