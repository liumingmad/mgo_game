#ifndef BODY_H
#define BODY_H

#include <string>
#include "json.hpp"

struct Request
{
    std::string action;
    std::string data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Request, action, data)

struct Response
{
    std::string action;
    std::string data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Response, action, data)



#endif // BODY_H