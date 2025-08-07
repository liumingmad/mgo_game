#include "ConfigManager.h"
#include "Log.h"
#include <fstream>
#include <mutex>
#include <cstdlib>

std::unique_ptr<ConfigManager> ConfigManager::instance = nullptr;
std::mutex ConfigManager::mutex_;

ConfigManager* ConfigManager::getInstance() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance == nullptr) {
        instance = std::unique_ptr<ConfigManager>(new ConfigManager());
    }
    return instance.get();
}

bool ConfigManager::loadConfig(const std::string& config_file) {
    try {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            LOG_ERROR("Cannot open config file: {}", config_file);
            return false;
        }

        file >> config_data;
        LOG_INFO("Config loaded successfully from: {}", config_file);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config: {}", e.what());
        return false;
    }
}

ConfigManager::MySQLConfig ConfigManager::getMySQLConfig() const {
    MySQLConfig config;
    
    // 优先使用环境变量，然后使用配置文件
    const char* env_host = std::getenv("MYSQL_HOST");
    const char* env_port = std::getenv("MYSQL_PORT");
    const char* env_user = std::getenv("MYSQL_USERNAME");
    const char* env_pass = std::getenv("MYSQL_PASSWORD");
    const char* env_db = std::getenv("MYSQL_DATABASE");

    config.host = env_host ? env_host : get<std::string>("/database/mysql/host", "127.0.0.1");
    config.port = env_port ? std::stoi(env_port) : get<int>("/database/mysql/port", 3306);
    config.username = env_user ? env_user : get<std::string>("/database/mysql/username", "root");
    config.password = env_pass ? env_pass : get<std::string>("/database/mysql/password", "123456");
    config.database = env_db ? env_db : get<std::string>("/database/mysql/database", "mgo");
    config.pool_size = get<int>("/database/mysql/pool_size", 10);
    config.connection_timeout = get<int>("/database/mysql/connection_timeout", 5);

    return config;
}

ConfigManager::RedisConfig ConfigManager::getRedisConfig() const {
    RedisConfig config;
    
    // 优先使用环境变量，然后使用配置文件
    const char* env_host = std::getenv("REDIS_HOST");
    const char* env_port = std::getenv("REDIS_PORT");
    const char* env_pass = std::getenv("REDIS_PASSWORD");

    config.host = env_host ? env_host : get<std::string>("/cache/redis/host", "127.0.0.1");
    config.port = env_port ? std::stoi(env_port) : get<int>("/cache/redis/port", 6379);
    config.password = env_pass ? env_pass : get<std::string>("/cache/redis/password", "");
    config.pool_size = get<int>("/cache/redis/pool_size", 3);
    config.wait_timeout_ms = get<int>("/cache/redis/wait_timeout_ms", 200);
    config.connection_lifetime_minutes = get<int>("/cache/redis/connection_lifetime_minutes", 10);

    return config;
}

ConfigManager::ServerConfig ConfigManager::getServerConfig() const {
    ServerConfig config;
    
    const char* env_port = std::getenv("SERVER_PORT");
    const char* env_host = std::getenv("SERVER_HOST");

    config.host = env_host ? env_host : get<std::string>("/server/host", "0.0.0.0");
    config.port = env_port ? std::stoi(env_port) : get<int>("/server/port", 8080);

    return config;
}