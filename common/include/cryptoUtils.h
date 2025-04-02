#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H


#include <string>

const std::string SECRET = "#ksl@lsl9m94lw!";

std::string base64_encode(std::string text);
std::string base64_decode(std::string text);

std::string aes_encode(std::string plain, std::string key);
std::string aes_decode(std::string encoded, std::string key);

std::string strRand(int len);

std::string generate_jwt(const std::string& user_id);
bool validate_jwt(const std::string& token);

#endif // CRYPTOUTILS_H