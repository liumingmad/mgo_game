#ifndef REDIS_CONF_H
#define REDIS_CONF_H

#include <sw/redis++/redis++.h>
#include "ConfigManager.h"

using namespace sw::redis;

class RedisPool {
private:
    Redis* redis;
    RedisPool() {

    }
    ~RedisPool() {}

public:
    RedisPool(RedisPool& pool) = delete;
    RedisPool& operator=(RedisPool& pool) = delete;
    static RedisPool& getInstance() {
        static RedisPool pool;
        return pool;
    }
    
    bool init() {
        auto config = ConfigManager::getInstance()->getRedisConfig();
        
        ConnectionOptions opts;
        opts.host = config.host;
        opts.port = config.port;
        if (!config.password.empty()) {
            opts.password = config.password;
        }

        ConnectionPoolOptions pool_opts;
        pool_opts.size = config.pool_size;
        pool_opts.wait_timeout = std::chrono::milliseconds(config.wait_timeout_ms);
        pool_opts.connection_lifetime = std::chrono::minutes(config.connection_lifetime_minutes);

        redis = new Redis(opts, pool_opts);
        return true;
    }

    Redis& getRedis() {
        return *redis;
    }


};


#endif // REDIS_CONF_H