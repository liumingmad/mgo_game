#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <iostream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include "db_connection_pool.h"
#include "global.h"
#include "Sgf.h"

std::shared_ptr<Player> query_user(std::string id) {
    std::shared_ptr<Player> player = std::make_shared<Player>();
    try
    {
        auto conn = DBConnectionPool::getInstance()->getConnection();
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::string sql = "SELECT id, username, level FROM users WHERE id=" + id + ";";
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(sql));
        while (res->next())
        {
            player->id = std::to_string(res->getInt("id"));
            player->name = res->getString("username");
            player->level = res->getInt("level");
            LOG_INFO("ID: {}, name:{}\n", player->id, player->name);
        }
    }
    catch (sql::SQLException &e)
    {
        LOG_ERROR("Error:{}\n", e.what());
        return nullptr;
    }
    return player;
}

bool saveSgf(std::shared_ptr<Sgf> sgf) {
    try {
        auto conn = DBConnectionPool::getInstance()->getConnection();

        std::string insertSQL = "INSERT INTO sgf (b_level, w_level, b_name, w_name, sgf, time_ms) VALUES (?, ?, ?, ?, ?, ?)";
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(insertSQL));

        pstmt->setInt(1, sgf->bLevel);                            // b_level
        pstmt->setInt(2, sgf->wLevel);                            // w_level
        pstmt->setString(3, sgf->bName);                    // b_name
        pstmt->setString(4, sgf->wName);                    // w_name
        pstmt->setString(5, sgf->sgf);        // sgf
        pstmt->setInt64(6, sgf->timeMs);

        pstmt->executeUpdate();
    }
    catch (sql::SQLException &e)
    {
        LOG_ERROR("Error:{}\n", e.what());
    }
    return false;
}

#endif // DB_UTILS_H