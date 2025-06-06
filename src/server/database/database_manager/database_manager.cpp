/*
 * This file is part of Royal Blocks.
 *
 * Royal Blocks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Royal Blocks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Royal Blocks.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "database_manager.hpp"

#include <filesystem>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <string_view>
#include <vector>

#include "../../../common/types/types.hpp"

std::string DatabaseManager::getDatabasePath(std::string_view fileOrDirectoryName) const {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (!appdata) throw std::runtime_error("APPDATA not set");
    return std::string(appdata) + "\\royal-blocks\\" + std::string(fileOrDirectoryName);
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    const char* home = std::getenv("HOME");
    if (!home) throw std::runtime_error("HOME not set");
    return std::string(home) + "/.config/royal-blocks/" + fileOrDirectoryName.data();
#else
    throw std::runtime_error("Unsupported OS");
#endif
}


// ### Constructor ###
DatabaseManager::DatabaseManager() {
    std::string_view path = "data/users.db";

    std::filesystem::path dbPath(getDatabasePath(path));
    std::filesystem::path dataPath = dbPath.parent_path();
    if (!std::filesystem::exists(dataPath)) {
        std::filesystem::create_directories(dataPath);
    }

    if (sqlite3_open(getDatabasePath(path).data(), &db_) != SQLITE_OK) {
        std::cerr << "Error SQLite: " << sqlite3_errmsg(db_) << std::endl;
    }
}

// ### Destructor ###
DatabaseManager::~DatabaseManager() { sqlite3_close(db_); }

// ### Private methods ###
bool DatabaseManager::createTables(const std::string &sql) {
    char *errorMess = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMess)
        == SQLITE_OK) {
        return true;
    } else {
        std::cerr << "error (create Table): " << errorMess << std::endl;
        sqlite3_free(errorMess);
        return false;
    }
}

// ### Public methods ###
bool DatabaseManager::executeSqlChangeData(
    const std::string &sql, const std::vector<MultiType> &params) {
    // Prepare the SQL statement
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Bind the parameters
    for (size_t i = 0; i < params.size(); ++i) {
        if (std::holds_alternative<UserID>(params[i])) {
            sqlite3_bind_int(stmt, static_cast<int>(i + 1),
                             std::get<UserID>(params[i]));
        } else {
            sqlite3_bind_text(stmt, static_cast<int>(i + 1),
                              std::get<std::string>(params[i]).c_str(), -1,
                              SQLITE_STATIC);
        }
    }

    // Execute the SQL statement
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "SQL execution error: " << sqlite3_errmsg(db_)
                  << std::endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::executeSqlRecoveryInt(
    const std::string &sql, const std::vector<MultiType> &params,
    int &result) const {
    // Prepare the SQL statement
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Bind the parameters
    for (size_t i = 0; i < params.size(); ++i) {
        if (std::holds_alternative<UserID>(params[i])) {
            sqlite3_bind_int(stmt, static_cast<int>(i + 1),
                             std::get<UserID>(params[i]));
        } else {
            sqlite3_bind_text(stmt, static_cast<int>(i + 1),
                              std::get<std::string>(params[i]).c_str(), -1,
                              SQLITE_STATIC);
        }
    }

    // Execute the SQL statement
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0);
        success = true;
    } else {
        std::cerr << "SQL execution error: " << sqlite3_errmsg(db_)
                  << std::endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::executeSqlRecoveryString(
    const std::string &sql, const std::vector<MultiType> &params,
    std::string &result) const {
    // Prepare the SQL statement
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Bind the parameters
    for (size_t i = 0; i < params.size(); ++i) {
        if (std::holds_alternative<UserID>(params[i])) {
            sqlite3_bind_int(stmt, static_cast<int>(i + 1),
                             std::get<UserID>(params[i]));
        } else {
            sqlite3_bind_text(stmt, static_cast<int>(i + 1),
                              std::get<std::string>(params[i]).c_str(), -1,
                              SQLITE_STATIC);
        }
    }

    // Execute the SQL statement
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        success = true;
    } else {
        std::cerr << "SQL execution error: " << sqlite3_errmsg(db_)
                  << std::endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return success;
}

// ### Getters ###
sqlite3 *DatabaseManager::getDatabase() const { return db_; }

std::vector<std::pair<std::string, size_t>>
DatabaseManager::getRanking() const {
    // Prepare the SQL statement
    std::vector<std::pair<std::string, size_t>> ranking;
    std::string sql = "SELECT username, score FROM users ORDER BY score DESC";
    sqlite3_stmt *stmt;

    // Execute the SQL statement
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        // Get the data from the statement
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string username =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            size_t score = static_cast<size_t>(sqlite3_column_int(stmt, 1));

            ranking.push_back({username, score});
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Error in getting ranking." << std::endl;
    }
    return ranking;
}

bool DatabaseManager::findUserInDatabase(const std::string &table,
                                         const UserID &userID) {
    // Prepare the SQL statement
    std::string sql =
        "SELECT COUNT(*) FROM " + table + " WHERE user1 = ? OR user2 = ?";
    sqlite3_stmt *stmt;
    bool exists = false;

    // Execute the SQL statement
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userID);
        sqlite3_bind_int(stmt, 2, userID);

        // Get the data from the statement
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            exists = (count > 0);
        }
    }

    sqlite3_finalize(stmt);
    return exists;
}

std::vector<int> DatabaseManager::getVectorInfo(const std::string &sql,
                                                const UserID &userID) const {
    std::vector<int> result;
    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Erreur lors de la préparation de la requête : "
                  << sqlite3_errmsg(db_) << std::endl;
        return result;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, userID);
    sqlite3_bind_int(stmt, 2, userID);

    // Execute the SQL statement
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        UserID colUser1 = sqlite3_column_int(stmt, 0);
        UserID colUser2 = sqlite3_column_int(stmt, 1);

        UserID friendId = (colUser1 == userID) ? colUser2 : colUser1;
        result.push_back(friendId);
    }

    // Verify that the loop exited because the statement was finished
    if (rc != SQLITE_DONE) {
        std::cerr << "Erreur lors de l'exécution de la requête : "
                  << sqlite3_errmsg(db_) << std::endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return result;
}