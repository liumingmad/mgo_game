#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <memory>
#include "json.hpp"

using json = nlohmann::json;

class ConfigManager {
private:
    json config_data;
    static std::unique_ptr<ConfigManager> instance;
    static std::mutex mutex_;

    ConfigManager() = default;

public:
    ConfigManager(ConfigManager &other) = delete;
    void operator=(const ConfigManager &) = delete;

    static ConfigManager* getInstance();
    bool loadConfig(const std::string& config_file = "config/config.json");
    
    // MySQL配置
    struct MySQLConfig {
        std::string host;
        int port;
        std::string username;
        std::string password;
        std::string database;
        int pool_size;
        int connection_timeout;
        
        std::string getConnectionUrl() const {
            return "tcp://" + host + ":" + std::to_string(port);
        }
    };

    // Redis配置
    struct RedisConfig {
        std::string host;
        int port;
        std::string password;
        int pool_size;
        int wait_timeout_ms;
        int connection_lifetime_minutes;
    };

    // 服务器配置
    struct ServerConfig {
        std::string host;
        int port;
    };

    MySQLConfig getMySQLConfig() const;
    RedisConfig getRedisConfig() const;
    ServerConfig getServerConfig() const;
    
    // 通用获取配置方法
    template<typename T>
    T get(const std::string& key, const T& default_value = T{}) const {
        try {
            return config_data.at(json::json_pointer(key)).get<T>();
        } catch (...) {
            return default_value;
        }
    }
};

#endif // CONFIG_MANAGER_H