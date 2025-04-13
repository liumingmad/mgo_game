#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <iostream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include "db_connection_pool.h"
#include "global.h"
Player* query_user(std::string id) {
    Player* player = new Player();
    try
    {
        auto conn = DBConnectionPool::getInstance()->getConnection();
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::string sql = "SELECT id, username, level FROM users WHERE id=" + id + ";";
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(sql));
        while (res->next())
        {
            std::cout << "ID: " << res->getInt("id") << ", Name: " << res->getString("username") << std::endl;
            player->id = std::to_string(res->getInt("id"));
            player->name = res->getString("username");
            player->level = res->getInt("level");
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return nullptr;
    }
    return player;
}

#endif