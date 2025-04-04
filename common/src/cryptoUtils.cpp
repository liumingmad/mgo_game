#include <string>
#include <random>
#include <crypto++/cryptlib.h>
#include <crypto++/osrng.h>
#include <iostream>
#include <crypto++/files.h>
#include <crypto++/base64.h>
#include <crypto++/modes.h>
#include <crypto++/hex.h>
#include <jwt-cpp/jwt.h>

#include "cryptoUtils.h"

using namespace CryptoPP;


std::string base64_encode(std::string plainText) {
	std::string encoded;
	CryptoPP::Base64Encoder encoder;
	encoder.Put((CryptoPP::byte *)plainText.data(), plainText.length());
	encoder.MessageEnd();

	CryptoPP::word64 size = encoder.MaxRetrievable();
	if (size)
	{
		encoded.resize(size);
		encoder.Get((CryptoPP::byte *)&encoded[0], encoded.size());
	}
    return encoded;
}

std::string base64_decode(std::string encoded) {
	std::string decoded;
	CryptoPP::Base64Decoder decoder;
	decoder.Put((CryptoPP::byte *)encoded.data(), encoded.size());
	decoder.MessageEnd();

	CryptoPP::word64 size = decoder.MaxRetrievable();
	if (size && size <= SIZE_MAX)
	{
		decoded.resize(size);
		decoder.Get((CryptoPP::byte *)&decoded[0], decoded.size());
	}
    return decoded;
}


std::string aes_encode(std::string plain, std::string key) {
    std::string cipher, encoded;
    try {
        // 採用 AES-ECB 加密
        ECB_Mode<AES>::Encryption e;

        // 設定金鑰
        e.SetKey((CryptoPP::byte*)key.c_str(), key.size());

        // 進行加密
        StringSource s(plain, true,
            new StreamTransformationFilter(e,
                new StringSink(cipher)
            ) // StreamTransformationFilter
        ); // StringSource

    } catch(const Exception& e) {
        std::cerr << e.what() << std::endl;
        return NULL;
    }

    // Pretty print cipher text
    // StringSource ss2( cipher, true,
    //     new HexEncoder(
    //         new StringSink( encoded )
    //     ) // HexEncoder
    // ); // StringSource
    // std::cout << "cipher text: " << encoded << std::endl;
    return cipher;
}

std::string aes_decode(std::string cipher, std::string key) {
    // CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH] = "abcd1234";

    std::string recovered;
    try {
        // 採用 AES-ECB 解密
        ECB_Mode<AES>::Decryption d;

        // 設定金鑰與 IV
        d.SetKey((CryptoPP::byte*)key.c_str(), key.size());

        // 進行解密
        StringSource s(cipher, true,
            new StreamTransformationFilter(d,
                new StringSink(recovered)
            ) // StreamTransformationFilter
        ); // StringSource

    } catch(const Exception& e) {
        std::cerr << e.what() << std::endl;
        return NULL;
    }

    // std::cout << "解開的明文：" << recovered << std::endl;
    return recovered;
}

std::string strRand(int length) {			// length: 产生字符串的长度
    char tmp;							// tmp: 暂存一个随机数
    std::string buffer;						// buffer: 保存返回值
    
    // 下面这两行比较重要:
    std::random_device rd;					// 产生一个 std::random_device 对象 rd
    std::default_random_engine random(rd());	// 用 rd 初始化一个随机数发生器 random
    
    for (int i = 0; i < length; i++) {
        tmp = random() % 36;	// 随机一个小于 36 的整数，0-9、A-Z 共 36 种字符
        if (tmp < 10) {			// 如果随机数小于 10，变换成一个阿拉伯数字的 ASCII
            tmp += '0';
        } else {				// 否则，变换成一个大写字母的 ASCII
            tmp -= 10;
            tmp += 'A';
        }
        buffer += tmp;
    }
    return buffer;
}

// 生成 JWT
std::string generate_jwt(const std::string& user_id) {
    auto token = jwt::create()
        .set_issuer("ming")                     // 签发者
        .set_subject("auth")                         // 用途
        .set_audience("mgo_app")                  // 接收方
        .set_payload_claim("user_id", jwt::claim(user_id)) // 自定义声明
        .set_issued_at(std::chrono::system_clock::now())    // 签发时间
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(365*24)) // 24小时后过期
        .sign(jwt::algorithm::hs256{SECRET}); // HS256 算法签名
    return token;
}
bool validate_jwt(const std::string& token) {
    try {
        // 解码 Token
        auto decoded = jwt::decode(token);

        // 创建验证器
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET}) // 校验算法和密钥
            .with_issuer("ming")    // 必须匹配签发者
            .with_audience("mgo_app"); // 必须匹配接收方

        verifier.verify(decoded); // 验证签名和有效期
        return true;
    } catch (const jwt::error::token_verification_exception& e) {
        std::cerr << "Token 验证失败: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return false;
    }
}

std::string extract_user_id(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        return decoded.get_payload_claim("user_id").as_string();
    } catch (const std::exception& e) {
        throw std::runtime_error("提取 user_id 失败: " + std::string(e.what()));
    }
}