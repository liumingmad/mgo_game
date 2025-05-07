#include <cstddef>
#include <string>
#include <optional>
#include "http_utils.h"

size_t WriteCallback(char *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

std::optional<User> check_token(std::string token)
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (!curl)
        return std::nullopt;

    std::string url = "http://host.docker.internal/api/users/me";

    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_PORT, 8080);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    struct curl_slist *headers = NULL;
    token = "Authorization:Bearer " + token;
    headers = curl_slist_append(headers, token.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    // {"code":200,"message":"success","data":{"id":22,"username":"ming","email":"1ming@gmail.com","password":null,"role":"USER"}}
    if (res != CURLE_OK)
    {
        std::cerr << "请求失败：" << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    try
    {
        nlohmann::json j = nlohmann::json::parse(responseData);
        HttpResponse response = j.get<HttpResponse>();
        if (response.code == 200)
        {
            return response.data;
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }
    catch (const nlohmann::json::type_error &e)
    {
        std::cerr << "JSON Type Error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Other Error: " << e.what() << std::endl;
    }

    return std::nullopt;
}