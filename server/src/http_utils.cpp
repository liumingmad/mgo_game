#include <cstddef>
#include <string>
#include <optional>
#include "http_utils.h"
#include "Log.h"

HttpClient http;

std::optional<User> check_token(std::string jwt_token)
{
    std::string url = "http://host.docker.internal/api/users/me";
    auto result = http.get(url, jwt_token);
    if (!result.has_value()) {
        return std::nullopt;
    } 

    try
    {
        nlohmann::json j = nlohmann::json::parse(result.value());
        HttpResponse response = j.get<HttpResponse>();
        if (response.code == 200)
        {
            return response.data;
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        LOG_ERROR("JSON Parse Error: {}\n", e.what());
    }
    catch (const nlohmann::json::type_error &e)
    {
        LOG_ERROR("JSON Type Error: {}\n", e.what());
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Other Error: {}\n", e.what());
    }

    return std::nullopt;
}
