#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <crypto++/cryptlib.h>
#include <crypto++/osrng.h>
#include <iostream>
#include <crypto++/files.h>
#include <crypto++/base64.h>
#include <crypto++/modes.h>
#include <crypto++/hex.h>
#include <string>


std::string base64_encode(std::string text);
std::string base64_decode(std::string text);

std::string aes_encode(std::string plain, std::string key);
std::string aes_decode(std::string encoded, std::string key);

std::string strRand(int len);

#endif // CRYPTOUTILS_H