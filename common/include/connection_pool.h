#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H


#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <mysql_driver.h>
#include <mysql_connection.h>

class ConnectionPool {
public:
    static ConnectionPool* getInstance() {
        static ConnectionPool instance;
        return &instance;
    }

    std::shared_ptr<sql::Connection> getConnection() {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        // 等待可用连接（带超时机制）
        if (!m_cond.wait_for(lock, std::chrono::seconds(5), 
            [this]{ return !m_connections.empty(); })) {
            throw sql::SQLException("Get connection timeout");
        }
        
        auto conn = m_connections.front();
        m_connections.pop();
        return std::shared_ptr<sql::Connection>(
            conn, 
            [this](sql::Connection* conn) { 
                releaseConnection(conn); 
            });
    }

    void initPool(const std::string& url, 
                 const std::string& user, 
                 const std::string& password, 
                 const std::string& schema, 
                 int poolSize = 10) {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        
        for (int i = 0; i < poolSize; ++i) {
            sql::Connection* conn = driver->connect(url, user, password);
            conn->setSchema(schema);
            m_connections.push(conn);
        }
    }

private:
    ConnectionPool() = default;
    ~ConnectionPool() {
        while (!m_connections.empty()) {
            delete m_connections.front();
            m_connections.pop();
        }
    }

    void releaseConnection(sql::Connection* conn) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.push(conn);
        m_cond.notify_one();
    }

    std::queue<sql::Connection*> m_connections;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

#endif // CONNECTION_POOL_H