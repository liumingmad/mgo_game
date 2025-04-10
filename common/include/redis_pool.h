#ifndef REDIS_CONF_H
#define REDIS_CONF_H

#include <sw/redis++/redis++.h>

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
        ConnectionOptions opts;
        opts.host = "172.17.0.1";
        opts.port = 6379;
        opts.password = "123456";

        ConnectionPoolOptions pool_opts;
        pool_opts.size = 3;  // 设置连接池大小
        pool_opts.wait_timeout = std::chrono::milliseconds(200);  // 可选，设置等待超时时间
        pool_opts.connection_lifetime = std::chrono::minutes(10);  // 可选，设置连接生命周期

        redis = new Redis(opts, pool_opts);
        return true;
    }

    Redis& getRedis() {
        return *redis;
    }


};


#endif // REDIS_CONF_H