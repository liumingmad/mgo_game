#include "HttpClient.h"
#include <iostream>

HttpClient::HttpClient() {
    static bool initialized = false;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        atexit([]() {
            curl_global_cleanup();
        });
        initialized = true;
    }
}

HttpClient::~HttpClient() {}

void HttpClient::setTimeout(long seconds) {
    timeoutSeconds = seconds;
}

size_t HttpClient::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    std::string *data = static_cast<std::string *>(userp);
    size_t totalSize = size * nmemb;
    data->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

std::optional<std::string> HttpClient::get(const std::string &url, const std::string &token) {
    CURL *curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_PORT, 8080);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    struct curl_slist *headers = nullptr;
    if (!token.empty()) {
        std::string authHeader = "Authorization: Bearer " + token;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "GET error: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }
    return response;
}

std::optional<std::string> HttpClient::post(const std::string &url, const std::string &jsonBody, const std::string &token) {
    CURL *curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_PORT, 8080);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonBody.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!token.empty()) {
        std::string authHeader = "Authorization: Bearer " + token;
        headers = curl_slist_append(headers, authHeader.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "POST error: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }
    return response;
}