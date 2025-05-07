#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <curl/curl.h>
#include <iostream>
#include <string>
#include "json.hpp"

//"{\"code\":200,\"message\":\"success\",\"data\":{\"id\":22,\"username\":\"ming\",\"email\":\"1ming@gmail.com\",\"password\":null,\"role\":\"USER\"}}"
struct User
{
    int id;
    std::string username;
    std::string email;
    // std::string password;
    std::string role;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, id, username, email, role)

struct HttpResponse
{
    int code;
    std::string message;
    User data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HttpResponse, code, message, data)

size_t WriteCallback(char *ptr, size_t size, size_t nmemb, std::string *data);

std::optional<User> check_token(std::string token);

#endif