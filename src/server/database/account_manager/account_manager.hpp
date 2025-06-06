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

#ifndef ACCOUNT_MANAGER_HPP
#define ACCOUNT_MANAGER_HPP

#include <memory>
#include <stddef.h>
#include <string>
#include <utility>
#include <vector>

#include "../../../common/types/types.hpp"

class DatabaseManager;

/**
 * @brief Status of the account creation (success, failure, etc.)
 *
 */
enum class CreateAccountStatus {
    FAILED,
    USERNAME_EXISTS,
    SUCCESS,
};

class AccountManager {
  private:
    std::shared_ptr<DatabaseManager> dbManager_;

    /*
     * @brief Check if a friendship exists between two users
     *
     * @param user Username of the first user
     * @param friendUser Username of the second user
     * @return true if the friendship exists
     */
    bool checkFriendshipExists(const std::string &user,
                               const std::string &friendUser) const;

  public:
    /*
     * @brief Construct a new Account Manager object
     *
     * @param dbPath Path to the database file
     */
    AccountManager(std::shared_ptr<DatabaseManager> &db);

    /*
     * @brief Destroy the Account Manager object
     */
    ~AccountManager() = default;

    /*
     * @brief Create a new account
     *
     * @param username Username of the account
     * @param password Password of the account
     * @return true if the account was created successfully
     */
    CreateAccountStatus createAccount(const std::string &username,
                                      const std::string &password);

    /*
     * @brief Delete an account
     *
     * @param userId ID of the account
     * @return true if the account was deleted successfully
     */
    bool deleteAccount(const UserID &userID);

    /*
     * @brief Check if the user already exists
     *
     * @param username Username to check
     * @return true if the user exists
     */
    bool checkUsernameExists(const std::string &username) const;

    /*
     * @brief Check if the password is correct for a user
     *
     * @param username Username of the user
     * @param password Password to check
     * @return true if the password is correct
     */
    bool checkUserPassword(const std::string &username,
                           const std::string &password) const;

    /*
     * @brief Login to an account
     *
     * @param username Username of the account
     * @param password Password of the account
     * @return true if the login was successful
     */
    bool login(const std::string &username, const std::string &password) const;

    /*
     * @brief Update the score of a user
     *
     * @param userID ID of the user
     * @param newScore New score of the user
     */
    void updateScore(const UserID &userID, const Score &newScore);

    bool updateUsername(const UserID &userID, std::string &newUsername);

    void updatePassword(const UserID &userID, std::string &newPassword);

    /*
     * @brief Get the user ID from the username
     *
     * @param username Username of the user
     * @return ID of the user
     */
    int getUserId(const std::string &username) const;

    /*
     * @brief Get the username from the user ID
     *
     * @param userID ID of the user
     * @return Username of the user
     */
    std::string getUsername(const UserID &userID) const;

    /*
     * @brief Get the ranking of the users for the endless mode
     *
     * @return Vector of pairs containing the username and the score
     */
    std::vector<std::pair<std::string, size_t>> getRanking() const;

    /*
     * @brief Get the password hash of a user
     *
     * @param nickname Username of the user
     * @return Password hash of the user
     */
    std::string getUserPasswordHash(const std::string &nickname) const;
};

#endif // ACCOUNT_MANAGER_HPP
