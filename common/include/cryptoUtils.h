#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H


#include <string>


std::string base64_encode(std::string text);
std::string base64_decode(std::string text);

std::string aes_encode(std::string plain, std::string key);
std::string aes_decode(std::string encoded, std::string key);

std::string strRand(int len);

#endif // CRYPTOUTILS_H