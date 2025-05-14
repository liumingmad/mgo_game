#pragma once
#include <string>
#include <optional>
#include <curl/curl.h>

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    std::optional<std::string> get(const std::string &url, const std::string &token = "");
    std::optional<std::string> post(const std::string &url, const std::string &jsonBody, const std::string &token = "");

    void setTimeout(long seconds); // 设置超时
private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    long timeoutSeconds = 5; // 默认超时 5 秒
};